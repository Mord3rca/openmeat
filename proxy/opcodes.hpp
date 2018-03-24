#ifndef OPCODES_HPP
#define OPCODES_HPP

/*
 * Lots of stuff here ! Have fun reading.
 */

namespace dm
{
  enum opcode
  {
    DISCONNECT  = 0x1a32,
    
    KEEP_ALIVE  = 0x1c06,
    
    MOVE        = 0x6902,
    PERK        = 0x6906,
    USE         = 0x690a,
    ATTACK      = 0x690d,
    USE_OBJECT  = 0x6910,
    SWAP_WEAPON = 0x6914,
    CRAFT       = 0x6952,
    TRAVEL      = 0x694b,
    
    PARRY = 0x8015, //Followed by 01 or 00
    
    ERROR = 0xffff
  };
  
  enum TRAVEL_DEST
  {
    SURVIVOR_PLACE  = 0x0001,
    LAKEVIEW_LANE   = 0x0002,
    MAZON_COLLEGE   = 0x000a,
    SUNSET_MALL     = 0x000b,
    WALKER_RIVER    = 0x000f,
    SACRAMENTO      = 0x0026,
    ARIZONNA_MUSEUM = 0x0027,
    BODEGA_BAY      = 0x002b,
    DOWNTOWN_SR     = 0x002e,
    HIGHWAY99       = 0x0031,
    BLUE_MESA       = 0x003f  //HL3 confirmed ?
  };
  
  enum PERKS
  {
    RUN       = 0x05, //Followed by 01 (run) or 00 (walk)
    MODIFIER  = 0x0b
  };
  
  enum MODIFIERS
  {
    RECUPERATOR   = 0x01,
    TIRELESS      = 0x02,
    ECONOMICAL    = 0x03,
    COLLECTIONNER = 0x04,
    MULE          = 0x05,
    INDEPENDENT   = 0x06
  };
  
  enum CRAFTS
  {
    RECYCLE_REQUEST = 0x09,
    RECYCLE_DO      = 0x0a,
    CREATE_ITEM     = 0x0c,
    RECYCLE_CONFIRM = 0x18
  };
  
  enum CRAFT_ITEM
  {
    FRUIT_SALAD   = 0x0125,
    KITCHEN_KNIFE = 0x0027,
    COMBAT_KNIFE  = 0x0139,
    ENERGY_DRINK  = 0x011b  //Be extreme.... Have a RedBull
  };
}

#endif //OPCODES_HPP
