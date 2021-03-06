#include "ObjectFactory.h"

#include "objects/MainObject.h"
#include "objects/OnMapObject.h"
#include "Game.h"
#include "Map.h"
#include "SyncRandom.h"
#include "objects/Creator.h"
#include "net/MagicStrings.h"
#include "net/NetworkMessagesTypes.h"

ObjectFactory::ObjectFactory()
{
    objects_table_.resize(100);
    id_ = 1;
    is_world_generating_ = true;

}

void ObjectFactory::UpdateProcessingItems()
{   
    if (!add_to_process_.size())
    {
        return;
    }
    for (auto it = add_to_process_.begin(); it != add_to_process_.end(); ++it)
    {
        if (!(*it).valid())
        {
            continue;
        }
        if ((*it)->GetFreq() == 0)
        {
            continue;
        }
        auto to_add = std::find(process_table_.begin(), process_table_.end(), *it);
        if (to_add == process_table_.end())
        {
            process_table_.push_back(*it);
        }
    }
    std::sort(process_table_.begin(), process_table_.end(),
    [](id_ptr_on<IMainObject> item1, id_ptr_on<IMainObject> item2)
    {
        return item1.ret_id() < item2.ret_id();
    });
    add_to_process_.clear();
}

void ObjectFactory::ForeachProcess()
{
    UpdateProcessingItems();

    size_t table_size = process_table_.size();
    for (size_t i = 0; i < table_size; ++i)
    {
        if (!(   process_table_[i].valid()
              && process_table_[i]->GetFreq()))
        {
            continue;
        }
        else if ((MAIN_TICK % process_table_[i]->GetFreq()) == 0)
        {
            process_table_[i]->Process();
        }
    }
}

void ObjectFactory::SaveMapHeader(std::stringstream& savefile)
{
    savefile << MAIN_TICK << std::endl;
    savefile << id_ << std::endl;
    savefile << GetMob().ret_id() << std::endl;

    // Random save
    savefile << random_helpers::get_seed() << std::endl;
    savefile << random_helpers::get_calls_counter() << std::endl;

    // Save Map Size

    savefile << GetMap().GetMapW() << std::endl;
    savefile << GetMap().GetMapH() << std::endl;
    savefile << GetMap().GetMapD() << std::endl;

    // Save player table
    savefile << players_table_.size() << " ";
    for (auto it = players_table_.begin(); it != players_table_.end(); ++it)
    {
        savefile << it->first << " ";
        savefile << it->second << " ";
    }
    savefile << std::endl;
}

void ObjectFactory::LoadMapHeader(std::stringstream& savefile, size_t real_this_mob)
{
    savefile >> MAIN_TICK;
    SYSTEM_STREAM << "MAIN_TICK: " << MAIN_TICK << std::endl;

    savefile >> id_;
    SYSTEM_STREAM << "id_: " << id_ << std::endl;
    
    size_t loc;
    savefile >> loc;
    SYSTEM_STREAM << "thisMob: " << loc << std::endl;
    
    unsigned int new_seed;
    unsigned int new_calls_counter;
    savefile >> new_seed;
    savefile >> new_calls_counter;

    random_helpers::set_rand(new_seed, new_calls_counter);

    objects_table_.resize(id_ + 1);

    // Load map size
    int x;
    int y;
    int z;

    savefile >> x;
    savefile >> y;
    savefile >> z;

    GetMap().ResizeMap(x, y, z);

    // Load player table
    size_t s;
    savefile >> s;
    for (size_t i = 0; i < s; ++i)
    {
        size_t first;
        savefile >> first;
        size_t second;
        savefile >> second;

        qDebug() << first;
        qDebug() << second;
        SetPlayerId(first, second);
    }

    qDebug() << "This mob: " << real_this_mob;

    if (real_this_mob == 0)
        SetMob(loc);
    else
        SetMob(GetPlayerId(real_this_mob));
}

void ObjectFactory::SaveMap(const char* path)
{
    std::fstream rfile;
    rfile.open(path, std::ios_base::out | std::ios_base::trunc);
    if(rfile.fail()) 
    {
        SYSTEM_STREAM << "Error open " << path << std::endl; 
        return;
    }
    std::stringstream savefile;
    SaveMap(savefile);
    rfile << savefile.str();
    rfile.close();
}
void ObjectFactory::SaveMap(std::stringstream& savefile)
{
    SaveMapHeader(savefile);
    auto it = ++objects_table_.begin();
    while(it != objects_table_.end())
    {
        if(*it) 
        {
            (*it++)->saveSelf(savefile);
            savefile << std::endl;
        }
        else
        {
            ++it;
        }
    }
    savefile << "0 ~";
}

void ObjectFactory::LoadMap(const char* path)
{
    SYSTEM_STREAM << path << std::endl;
    std::stringstream savefile;
    std::fstream rfile;
    rfile.open(path, std::ios_base::in);

    SYSTEM_STREAM << "Point 1" << std::endl;

    rfile.seekg (0, std::ios::end);
    std::streamoff length = rfile.tellg();
    rfile.seekg (0, std::ios::beg);
    char* buff = new char[static_cast<size_t>(length)];

    rfile.read(buff, length);
    rfile.close();
    savefile.write(buff, length);
    delete[] buff;
    SYSTEM_STREAM << "End map load" << std::endl;
    LoadMap(savefile, 0);
}

const int AVERAGE_BYTE_PER_TILE = 129 * 2;
const long int UNCOMPRESS_LEN_DEFAULT = 50 * 50 * 5 * AVERAGE_BYTE_PER_TILE;
void ObjectFactory::LoadMap(std::stringstream& savefile, size_t real_this_mob)
{
    ClearMap();

    LoadMapHeader(savefile, real_this_mob);
    int j = 0;
    while(!savefile.eof())
    {
        j++;
        if(savefile.fail())
        {
            SYSTEM_STREAM << "Error! " << j << "\n";
            SDL_Delay(10000);
        }
        std::string type;
        savefile >> type;
        if(type == "0")
        {
            SYSTEM_STREAM << "Zero id reached" << std::endl;
            break;
        }

        //SYSTEM_STREAM << "Line number: " << j << std::endl;

        size_t id_loc;
        savefile >> id_loc;
        
        id_ptr_on<IMainObject> i;
        i = CreateVoid<IMainObject>(type, id_loc);
        i->loadSelf(savefile);
    }
    SYSTEM_STREAM << "\n NUM OF ELEMENTS CREATED: " << j << "\n";
    ChangeMob(GetMob());
    is_world_generating_ = false;
}

IMainObject* ObjectFactory::NewVoidObject(const std::string& type, size_t id)
{
    auto il = (*itemList());
    auto f = il[type];
    return f(id);
};

IMainObject* ObjectFactory::NewVoidObjectSaved(const std::string& type)
{
    return (*itemListSaved())[type]();
};

void ObjectFactory::ClearMap()
{
    size_t table_size = objects_table_.size();
    for (size_t i = 1; i < table_size; ++i)
        if (objects_table_[i] != nullptr)
            objects_table_[i]->delThis();
    if (table_size != objects_table_.size())
        SYSTEM_STREAM << "WARNING: table_size != idTable_.size()!" << std::endl;

    process_table_.clear();
    add_to_process_.clear();
    players_table_.clear();

    id_ = 1;
}

void ObjectFactory::BeginWorldCreation()
{
    is_world_generating_ = true;
}

void ObjectFactory::FinishWorldCreation()
{
    is_world_generating_ = false;
    size_t table_size = objects_table_.size();
    for (size_t i = 1; i < table_size; ++i)
    {
        if (objects_table_[i] != nullptr)
        {
            objects_table_[i]->AfterWorldCreation();
        }
    }
}

unsigned int ObjectFactory::Hash()
{
    unsigned int h = 0;
    size_t table_size = objects_table_.size();
    for (size_t i = 1; i < table_size; ++i)
    {
        if (objects_table_[i] != nullptr)
        {
            h += objects_table_[i]->hashSelf();
        }
    }

    ForceManager::Get().Clear();
    h += ForceManager::Get().Hash();

    ClearProcessing();

    int i = 1;
    for (auto p = process_table_.begin(); p != process_table_.end(); ++p)
    {
        h += p->ret_id() * i;
        i++;
    }

    return h;
}

void ObjectFactory::SetPlayerId(size_t net_id, size_t real_id)
{
    players_table_[net_id] = real_id;
}
size_t ObjectFactory::GetPlayerId(size_t net_id)
{
    auto it = players_table_.find(net_id);
    if (it != players_table_.end())
    {
        return it->second;
    }
    return 0;
}

size_t ObjectFactory::GetNetId(size_t real_id)
{
    for (auto it = players_table_.begin(); it != players_table_.end(); ++it)
        if (it->second == real_id)
            return it->first;
    return 0;
}

void ObjectFactory::AddProcessingItem(id_ptr_on<IMainObject> item)
{
    add_to_process_.push_back(item);
}

void ObjectFactory::ClearProcessing()
{
    std::vector<id_ptr_on<IMainObject>> remove_from_process;
    size_t table_size = process_table_.size();
    for (size_t i = 0; i < table_size; ++i)
    {
        if (!(   process_table_[i].valid()
              && process_table_[i]->GetFreq()))
        {
            remove_from_process.push_back(process_table_[i]);
        }
    }

    for (auto it = remove_from_process.begin(); it != remove_from_process.end(); ++it)
    {
        process_table_.erase(std::find(process_table_.begin(), process_table_.end(), *it));
    }
}

ObjectFactory* item_fabric_ = 0;
ObjectFactory& GetFactory()
{
    return *item_fabric_;
}
void SetFactory(ObjectFactory* item_fabric)
{
    item_fabric_ = item_fabric;
}
