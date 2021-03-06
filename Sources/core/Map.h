#pragma once

#include <list>
#include <vector>

#include "HelperStructers.h"
#include "Constheader.h"
#include "objects/Tile.h"
#include "Atmos.h"

class LOSfinder
{
public:
    std::list<point>* CalculateVisisble(std::list<point>* retval, int posx, int posy, int posz = 0);
private:
    void Clear();
    std::list<point> worklist;

};

class MapMaster
{
public:
    MapMaster();
    typedef id_ptr_on<CubeTile> SqType;
    std::vector<std::vector<std::vector<SqType> > > squares;

    Atmosphere atmosphere;

    void SaveToMapGen(const std::string& name);
    void LoadFromMapGen(const std::string& name);


    int GetMapW() const
    {
        return squares.size();
    }
    int GetMapH() const
    {
        return squares[0].size();
    }
    int GetMapD() const
    {
        return squares[0][0].size();
    }

    void ResizeMap(int x, int y, int z);

    void Represent();
    void GenerateFrame();

    bool IsTransparent(int posx, int posy, int posz = 0);

    
    void FillAtmosphere();

    void MakeTiles(int size_x, int size_y, int size_z);

    PassableLevel GetPassable(int posx, int posy, int posz = 0, Dir direct = D_ALL);

    static void switchDir(int& posx, int& posy, Dir direct, int num = 1, bool back = false);
    
    id_ptr_on<IOnMapObject> Click(int x, int y);
    
    LOSfinder losf;
};

bool IsMapValid();
MapMaster& GetMap();
void SetMapMaster(MapMaster* map_master);
