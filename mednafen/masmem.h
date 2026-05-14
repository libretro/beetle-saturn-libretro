#ifndef __MDFN_PSX_MASMEM_H
#define __MDFN_PSX_MASMEM_H

// TODO, WIP (big-endian stores and loads not fully supported yet)

#ifdef MSB_FIRST
#define MAS_NATIVE_IS_BIGENDIAN 1
#else
#define MAS_NATIVE_IS_BIGENDIAN 0
#endif

static INLINE uint16_t LoadU16_RBO(const uint16_t *a)
{
 #ifdef ARCH_POWERPC
  uint16_t tmp;

  __asm__ ("lhbrx %0, %y1" : "=r"(tmp) : "Z"(*a));

  return(tmp);

 #else
  uint16_t tmp = *a;
  return((tmp << 8) | (tmp >> 8));
 #endif
}

static INLINE uint32_t LoadU32_RBO(const uint32_t *a)
{
 #ifdef ARCH_POWERPC
  uint32_t tmp;

  __asm__ ("lwbrx %0, %y1" : "=r"(tmp) : "Z"(*a));

  return(tmp);
 #else
  uint32_t tmp = *a;
  return((tmp << 24) | ((tmp & 0xFF00) << 8) | ((tmp >> 8) & 0xFF00) | (tmp >> 24));
 #endif
}

static INLINE void StoreU16_RBO(uint16_t *a, const uint16_t v)
{
 #ifdef ARCH_POWERPC
  __asm__ ("sthbrx %0, %y1" : : "r"(v), "Z"(*a));
 #else
  uint16_t tmp = (v << 8) | (v >> 8);
  *a = tmp;
 #endif
}

static INLINE void StoreU32_RBO(uint32_t *a, const uint32_t v)
{
 #ifdef ARCH_POWERPC
  __asm__ ("stwbrx %0, %y1" : : "r"(v), "Z"(*a));
 #else
  uint32_t tmp = (v << 24) | ((v & 0xFF00) << 8) | ((v >> 8) & 0xFF00) | (v >> 24);
  *a = tmp;
 #endif
}

static INLINE uint16_t LoadU16_LE(const uint16_t *a)
{
#ifdef MSB_FIRST
   return LoadU16_RBO(a);
#else
   return *a;
#endif
}

static INLINE uint32_t LoadU32_LE(const uint32_t *a)
{
#ifdef MSB_FIRST
   return LoadU32_RBO(a);
#else
   return *a;
#endif
}

static INLINE void StoreU16_LE(uint16_t *a, const uint16_t v)
{
#ifdef MSB_FIRST
 StoreU16_RBO(a, v);
#else
 *a = v;
#endif
}

static INLINE void StoreU32_LE(uint32_t *a, const uint32_t v)
{
#ifdef MSB_FIRST
 StoreU32_RBO(a, v);
#else
 *a = v;
#endif
}


// address must not be >= size specified by template parameter, and address must be a multiple of the byte-size of the
// unit(1,2,4) being read(except for Read/WriteU24, which only needs to be byte-aligned).
//
// max_unit_type should be uint16_t or uint32_t
//
// pre_padding and post_padding are specified in units of sizeof(max_unit_type).
//
template<unsigned size, typename max_unit_type, bool big_endian> //, unsigned pre_padding_count, unsigned post_padding_count>
struct MultiAccessSizeMem
{
 //max_unit_type pre_padding[pre_padding_count ? pre_padding_count : 1];

 union
 {
  uint8_t data8[size];
  uint16_t data16[size / sizeof(uint16_t)];
  uint32_t data32[size / sizeof(uint32_t)];
 };

 //max_unit_type post_padding[post_padding_count ? post_padding_count : 1];

 INLINE uint8_t ReadU8(uint32_t address)
 {
  return data8[address];
 }

 INLINE uint16_t ReadU16(uint32_t address)
 {
  if(MAS_NATIVE_IS_BIGENDIAN == big_endian)
   return *(uint16_t*)(((uint8_t*)data16) + address);
  else
   return LoadU16_RBO((uint16_t*)(((uint8_t*)data16) + address));
 }

 INLINE uint32_t ReadU32(uint32_t address)
 {
  if(MAS_NATIVE_IS_BIGENDIAN == big_endian)
   return *(uint32_t*)(((uint8_t*)data32) + address);
  else
   return LoadU32_RBO((uint32_t*)(((uint8_t*)data32) + address));
 }

 INLINE uint32_t ReadU24(uint32_t address)
 {
  uint32_t ret;

  if(!big_endian)
  {
   ret = ReadU8(address) | (ReadU8(address + 1) << 8) | (ReadU8(address + 2) << 16);
  }

  return(ret);
 }


 INLINE void WriteU8(uint32_t address, uint8_t value)
 {
  data8[address] = value;
 }

 INLINE void WriteU16(uint32_t address, uint16_t value)
 {
  if(MAS_NATIVE_IS_BIGENDIAN == big_endian)
   *(uint16_t*)(((uint8_t*)data16) + address) = value;
  else
   StoreU16_RBO((uint16_t*)(((uint8_t*)data16) + address), value);
 }

 INLINE void WriteU32(uint32_t address, uint32_t value)
 {
  if(MAS_NATIVE_IS_BIGENDIAN == big_endian)
   *(uint32_t*)(((uint8_t*)data32) + address) = value;
  else
   StoreU32_RBO((uint32_t*)(((uint8_t*)data32) + address), value);
 }

 INLINE void WriteU24(uint32_t address, uint32_t value)
 {
  if(!big_endian)
  {
   WriteU8(address + 0, value >> 0);
   WriteU8(address + 1, value >> 8);
   WriteU8(address + 2, value >> 16);
  }
 }

 template<typename T>
 INLINE T Read(uint32_t address)
 {
  if(sizeof(T) == 4)
   return(ReadU32(address));
  else if(sizeof(T) == 2)
   return(ReadU16(address));
  else
   return(ReadU8(address));
 }

 template<typename T>
 INLINE void Write(uint32_t address, T value)
 {
  if(sizeof(T) == 4)
   WriteU32(address, value);
  else if(sizeof(T) == 2)
   WriteU16(address, value);
  else
   WriteU8(address, value);
 }
};

#undef MAS_NATIVE_IS_BIGENDIAN

#endif
