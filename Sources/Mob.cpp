#include "MapClass.h"
#include "Mob.h"
#include "LiquidHolder.h"
#include "Turf.h"

#include "NetClientImpl.h"
#include "EffectSystem.h"
#include "MoveEffect.h"
#include "sync_random.h"
#include "ItemFabric.h"
#include "MagicStrings.h"

void Manager::checkMove(Dir direct)
{
    //cautLastItem(direct);
    moveEach(direct);
};


void Manager::touchEach(Dir direct)
{
    IMainItem::mob->dMove = direct;
}

void Manager::moveEach(Dir direct)
{
    switch(direct)
    {
    case D_UP:
        for(int x = 0; x < sizeHmap; x++)
        {
            for(int y = 0; y < sizeWmap; y++)
            {
                auto itr = map->squares[x][y].begin();
                while(itr != map->squares[x][y].end())
                    (*itr++)->y += TITLE_SIZE;
            }
        }
        break;
    case D_DOWN:
        for(int x = 0; x < sizeHmap; x++)
        {
            for(int y = 0; y < sizeWmap; y++)
            {
                auto itr = map->squares[x][y].begin();
                while(itr != map->squares[x][y].end())
                    (*itr++)->y -= TITLE_SIZE;
            }
        }
        break;
    case D_LEFT:
        for(int x = 0; x < sizeHmap; x++)
        {
            for(int y = 0; y < sizeWmap; y++)
            {
                auto itr = map->squares[x][y].begin();
                while(itr != map->squares[x][y].end())
                    (*itr++)->x += TITLE_SIZE;
            }
        }
        break;
    case D_RIGHT:
        for(int x = 0; x < sizeHmap; x++)
        {
            for(int y = 0; y < sizeWmap; y++)
            {
                auto itr = map->squares[x][y].begin();
                while(itr != map->squares[x][y].end())
                    (*itr++)->x -= TITLE_SIZE;
            }
        }
        break;
    }
    undoCenterMove(direct);
};

void Manager::undoCenterMove(Dir direct)
{
    for(int x = max(0, thisMob->posx - sizeHsq); x <= min(thisMob->posx + sizeHsq, sizeHmap - 1); x++)
    {
        for(int y = max(0, thisMob->posy - sizeWsq); y <= min(thisMob->posy + sizeWsq, sizeWmap - 1); y++)
        {
            auto itr = map->squares[x][y].begin();
            while(itr != map->squares[x][y].end())
            {
                Move* eff = getEffectOf<Move>();
                eff->Init(TITLE_SIZE, direct, thisMob->pixSpeed, *itr);
                eff->Start();
                ++itr;
            }
        }
    }
};

void Manager::changeMob(id_ptr_on<IMob>& i)
{
    int oldposx = beginMobPosX, oldposy = beginMobPosY;
    if(IMainItem::mob)
    {
        oldposx = IMainItem::mob->posx;
        oldposy = IMainItem::mob->posy;
        delete IMainItem::mob;
    }
    IMainItem::mob = castTo<IMob>(ItemFabric::newVoidItemSaved(i->T_ITEM()));//TODO: repair //REPAIR WHAT?!

    thisMob = i.ret_id();
    thisMob->onMobControl = true;
    thisMob->thisMobControl = true;
    *IMainItem::mob = **thisMob;
    std::stringstream thisstr;
    IMainItem::map->centerFromTo(oldposx, oldposy, thisMob->posx, thisMob->posy);

    i->InitGUI();

    SYSTEM_STREAM << "\nTHIS MOB CHANGE: " << thisMob.ret_id() << " ";
};

Manager::Manager(Mode mode, std::string adrs)
{
    mode_ = mode;
    adrs_ = adrs;
    visiblePoint = new std::list<point>;
    isMove = false;
    done = 0;
    pause = 0;
    last_fps = FPS_MAX;
    net_client = NetClient::Init(this);
};
 
void Manager::process()
{

    map->numOfPathfind = 0;
    SDL_Color color = {255, 255, 255, 0};

    int delay = 0;
    int lastTimeFps = SDL_GetTicks();
    int lastTimeC = SDL_GetTicks();
    fps = 0;
    bool process_in = false;
    while(done == 0)
    { 
        *IMainItem::mob = **thisMob;
        if (!NODRAW)
        processInput();
        IMainItem::fabric->Sync();
        if(net_client->Ready())
        {
            process_in = true;
            process_in_msg();
            MAIN_TICK++;
        }
        if (thisMob.ret_id())
        *IMainItem::mob = **thisMob;

        if(process_in)
        {
            numOfDeer = 0;
            IMainItem::fabric->foreachProcess();
        }
        *IMainItem::mob = **thisMob;
         
        if (!NODRAW)
        {
            map->Draw();
            FabricProcesser::Get()->process();
        }
        *IMainItem::mob = **thisMob;
      
        //checkMoveMob();
        if (!NODRAW)
            thisMob->processGUI();

        texts.Process();

        if((SDL_GetTicks() - lastTimeFps) >= 1000)
        {
            visiblePoint->clear();
            visiblePoint = map->losf.calculateVisisble(visiblePoint, thisMob->posx, thisMob->posy, thisMob->level); 
            //SYSTEM_STREAM << loc << std::endl;
            if (!NODRAW)
            {
                //SDL_FreeSurface(sFPS);
                //sFPS = TTF_RenderText_Blended(map->aSpr.font, loc, color);
            }
            if(!(fps > FPS_MAX - 10 && fps < FPS_MAX - 10))
            delay = (int)(1000.0 / FPS_MAX + delay - 1000.0 / fps);
            lastTimeFps = SDL_GetTicks();
            last_fps = fps;
            fps = 0;
          

            map->numOfPathfind = 0;
        }
        ++fps;
        if(delay > 0) 
            SDL_Delay(delay);
        else 
            delay = 0;

        gl_screen->Swap();
        //++MAIN_TICK;
        process_in = false;
        if (net_client->Process() == false)
        {
            SYSTEM_STREAM << "Fail receive messages" << std::endl;
            SDL_Delay(10000);
            break;
        }
    }
    TTF_Quit();
    SDL_Quit();
};

void Manager::checkMoveMob()
{
};

#define SEND_KEY_MACRO(key) \
      if(keys[key]) \
      { \
          if(MAIN_TICK - lastShoot > 4) \
          { \
              Message msg; \
              msg.text = #key; \
              net_client->Send(msg); \
              lastShoot = (int)MAIN_TICK; \
          } \
      }

void Manager::processInput()
{
    static Uint8* keys;
    static int lastShoot = 0;
    SDL_Event event;    
    while (SDL_PollEvent(&event))
    { 
        if(event.type == SDL_QUIT) done = 1; 
        if(event.type == SDL_KEYUP)
        {
            if(event.key.keysym.sym == SDLK_o) pause = !pause;  
        }
        if(event.type == SDL_MOUSEBUTTONDOWN)
        {
            auto item = map->click(event.button.x, event.button.y);
            if (item.ret_id())
                last_touch = item->name;
        }
           
        keys = SDL_GetKeyState(NULL);
        SEND_KEY_MACRO(SDLK_UP);
        SEND_KEY_MACRO(SDLK_DOWN);
        SEND_KEY_MACRO(SDLK_LEFT);
        SEND_KEY_MACRO(SDLK_RIGHT);
        SEND_KEY_MACRO(SDLK_j);
        SEND_KEY_MACRO(SDLK_p);
        SEND_KEY_MACRO(SDLK_q);
        SEND_KEY_MACRO(SDLK_f);
        SEND_KEY_MACRO(SDLK_a);
        SEND_KEY_MACRO(SDLK_s);
        SEND_KEY_MACRO(SDLK_1);
        SEND_KEY_MACRO(SDLK_2);
        SEND_KEY_MACRO(SDLK_3);
        SEND_KEY_MACRO(SDLK_4);
        SEND_KEY_MACRO(SDLK_d);
        SEND_KEY_MACRO(SDLK_e);
        SEND_KEY_MACRO(SDLK_c);

        if(keys[SDLK_h])
        {
            int locatime = SDL_GetTicks();
            auto itr = map->squares[IMainItem::mob->posx][IMainItem::mob->posy].begin();
            int i = 0;
            while(itr != map->squares[IMainItem::mob->posx][IMainItem::mob->posy].end())
            {
                SYSTEM_STREAM << i <<": Level " << (*itr)->level;
                itr++;
                i++;
            };
            SYSTEM_STREAM << "Num item: " << i << " in " << (SDL_GetTicks() - locatime) * 1.0 / 1000 << " sec" << std::endl;
        }
        if(keys[SDLK_F5])
        {
            int locatime = SDL_GetTicks();
            IMainItem::fabric->saveMap(GetMode() == SERVER ? "servermap.map" : "clientmap.map");
            SYSTEM_STREAM << "Map saved in "<< (SDL_GetTicks() - locatime) * 1.0 / 1000 << " second" << std::endl;
        }
        if(keys[SDLK_F6])
        {
            int locatime = SDL_GetTicks();
            IMainItem::fabric->clearMap();
            IMainItem::fabric->loadMap(GetMode() == SERVER ? "servermap.map" : "clientmap.map");
            SYSTEM_STREAM << "Map load in " << (SDL_GetTicks() - locatime) * 1.0 / 1000 << " second" << std::endl;
        }
        if(keys[SDLK_h])
        {
            SYSTEM_STREAM << "World's hash: " << IMainItem::fabric->hash_all() << std::endl; 
        }
    }
};

void Manager::initWorld()
{
    tick_recv = 0;
    isMove = 0;
    Uint32 rmask, gmask, bmask, amask;
    SetMasks(&rmask, &gmask, &bmask, &amask);

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    { 
        SYSTEM_STREAM << "Unable to init SDL: " << SDL_GetError() << std::endl; 
        SDL_Delay(10000);
        return;
    }
    atexit(SDL_Quit);
    SYSTEM_STREAM << "Begin TTF init\n";
    SYSTEM_STREAM << TTF_Init() << " - return\n";
    SYSTEM_STREAM << " End TTF init\n";
    atexit(TTF_Quit);
    SYSTEM_STREAM << "Begin NET init\n";
    SYSTEM_STREAM << SDLNet_Init() << " - return\n";
    SYSTEM_STREAM << " End NET init\n";
    atexit(SDLNet_Quit);
    IMainItem::fabric = new ItemFabric;
    map = new MapMaster;
    SDL_WM_SetCaption("GOS", "GOS");
    if (!NODRAW)
        gl_screen = new Screen(sizeW, sizeH);



    map->screen = gl_screen;
    map->loManager = 0;
    IMainItem::map = map;
    IMainItem::mobMaster = this;
    
    map->mobi = this;

    srand(SDL_GetTicks());
    id_ptr_on<IMob> newmob;

    newmob = IMainItem::fabric->newItemOnMap<IMob>(hash("ork"), sizeHmap / 2, sizeWmap / 2);
    changeMob(newmob);
    newmob->x = beginMobPosX * TITLE_SIZE;
    newmob->y = beginMobPosY * TITLE_SIZE;

    auto tptr = IMainItem::fabric->newItemOnMap<IOnMapItem>(hash("Teleportator"), sizeHmap / 2, sizeWmap / 2);
    SetCreator(tptr.ret_id());

    map->makeMap();
    thisMob->passable = 1;
    LiquidHolder::LoadReaction();

    LoginData data;
    data.who = newmob.ret_id();
    data.word_for_who = 1;
    net_client->Connect(adrs_, DEFAULT_PORT, data);

    texts["FPS"].SetUpdater
    ([this](std::string* str)
    {
        std::stringstream ss; 
        ss << last_fps; 
        ss >> *str;
    }).SetFreq(1000).SetSize(20);

    texts["LastTouch"].SetUpdater
    ([this](std::string* str)
    {
        *str = last_touch;
    }).SetFreq(20).SetPlace(0, 400).SetSize(35);

    
};

void Manager::loadIniFile()
{

};

void Manager::process_in_msg()
{
    Message msg;
    while (true)
    {
        net_client->Recv(&msg);
        if (msg.text == Net::NEXTTICK)
            return;
        
        id_ptr_on<IMessageReceiver> i;
        i = msg.to;

        if (i.valid())
            i->processGUImsg(msg);
        else
            SYSTEM_STREAM << "Wrong id accepted - " << msg.to << std::endl;
    }
}

size_t Manager::GetCreator() const 
{
    return creator_;
}

void Manager::SetCreator(size_t new_creator) 
{
    creator_ = new_creator;
}

bool Manager::isMobVisible(int posx, int posy)
{
    // TODO: matrix for fast check
    if (visiblePoint == nullptr)
        return false;
    for (auto it = visiblePoint->begin(); it != visiblePoint->end(); ++it)
        if(it->posx == posx && it->posy == posy)
            return true;
    return false;
}


