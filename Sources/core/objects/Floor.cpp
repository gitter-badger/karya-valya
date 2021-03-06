#include "Floor.h"

#include "representation/Sound.h"

#include "Item.h"
#include "FloorTile.h"
#include "Pipes.h"

#include "../ObjectFactory.h"

Floor::Floor(size_t id) : ITurf(id)
{
    transparent = true;
    SetPassable(D_ALL, Passable::FULL);

    SetSprite("icons/floors.dmi");
    SetState("floor");

    name = "Floor";

    open_ = false;
    bloody = false;
}

void Floor::AfterWorldCreation()
{
    ITurf::AfterWorldCreation();
    SetOpen(open_);
}

void Floor::AttackBy(id_ptr_on<Item> item)
{
    if (id_ptr_on<Crowbar> c = item)
    {
        if (!open_)
        {
            SetOpen(true);
            GetFactory().Create<Item>(FloorTile::T_ITEM_S(), GetOwner());
            PlaySoundIfVisible("Crowbar.ogg", owner.ret_id());
        }
    }
    else if (id_ptr_on<FloorTile> ftile = item)
    {
        if (open_)
        {
            SetOpen(false);
            ftile->delThis();
            PlaySoundIfVisible("Deconstruct.ogg", owner.ret_id());
        }
    }
}

void Floor::SetOpen(bool o)
{
    open_ = o;
    if (open_)
    {
        SetState("plating");
        v_level = 0;
        if (auto vent = owner->GetItem<Vent>())
        {
            vent->SetHidden(false);
        }
    }
    else
    {
        SetState("floor");
        v_level = 2;
        if (auto vent = owner->GetItem<Vent>())
        {
            vent->SetHidden(true);
        }
    }
    GetView()->RemoveOverlays();
}


Plating::Plating(size_t id) : Floor(id)
{
    open_ = true;
    // For map editor
    SetState("plating");
}
