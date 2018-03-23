#ifndef OPCODES_HPP
#define OPCODES_HPP

namespace dm
{
  enum opcode
  {
    PERK        = 0x6906,
    USE         = 0x690a,
    SWAP_WEAPON = 0x6914,
    CRAFT       = 0x6952,
    TRAVEL      = 0x694b,
    
    PARRY = 0x8015, //Followed by 01 or 00
    
    ERROR = 0xffff
  };
  
  enum PERKS
  {
    RUN       = 0x05, //Followed by 01 (run) or 00 (walk)
    MODIFIER  = 0x0b
  };
  
  /*PERKS - MODIFIER List:
   * 01 - Recuperator
   * 02 - Tireless
   * 03 - Economical
   * 04 - Collectioner
   * 05 - Mule
   * 06 - Independent
  */
  
  enum CRAFTS
  {
    RECYCLE_REQUEST = 0x09,
    RECYCLE_DO      = 0x0a,
    CREATE_ITEM     = 0x0c
  }
  
  /* Craft Item List:
   * 0027 - Kitchen Knife
   * 0139 - Combat Knife
   * 011b - Energy Drink (Be extreme.... Have a RedBull)
  */
}

#endif //OPCODES_HPP
