//
// Keep everything in this file as C macros!
// These helpers allow to easily define packed types inside and outside of shader code.
// Getters are used in shader code while setters are on the C side.
// `SHADER_CODE` is defined only inside shaders.
//

#define PACKED_SIZE(size, shift)                    (((size) + ((1 << (shift)) - 1)) >> (shift))
#define PACKED_2(size)                              PACKED_SIZE(size, 1)  //  2 x int2/float2
#define PACKED_4(size)                              PACKED_SIZE(size, 2)  //  4 x int/float
#define PACKED_8(size)                              PACKED_SIZE(size, 3)  //  8 x short
#define PACKED_16(size)                             PACKED_SIZE(size, 4)  // 16 x byte

#define PACKED_INDEX_2(arr, index, subindex)        (arr[(index) >> 1][(((index) & 1) << 1) + (subindex)])
#define PACKED_INDEX_4(arr, index)                  (arr[(index) >> 2][(index) & 3])
#define PACKED_INDEX_8(arr, index)                  (arr[(index) >> 3][((index) >> 1) & 3])
#define PACKED_INDEX_16(arr, index)                 (arr[(index) >> 4][((index) >> 2) & 3])

#ifdef SHADER_CODE

#define PACKED_GET_2(arr, index, subindex)          PACKED_INDEX_2(arr, index, subindex)
#define PACKED_GET_4(arr, index)                    PACKED_INDEX_4(arr, index)
#define PACKED_GET_8(arr, index)                    ((PACKED_INDEX_8(arr, index)  >> (16 * ((index) & 1))) & 0xFFFF)
#define PACKED_GET_16(arr, index)                   ((PACKED_INDEX_16(arr, index) >> (8  * ((index) & 3))) & 0xFF)

#define UNPACK(value, shift, width) \
    (((value) >> (shift)) & ((1 << (width)) - 1))

#else

#define _INDEX_BITS_(index, shift)                  ((index) & ((1 << ((shift) - 2)) - 1))
#define _SHIFT_WIDTH_(shift)                        (32 >> ((shift) - 2))
#define _SHIFT_BITS_(shift)                         ((1 << _SHIFT_WIDTH_(shift)) - 1)

#define UNSET_BITS(arrindex, index, shift) \
    ((arrindex) & ~_SHIFTL(_SHIFT_BITS_(shift), _SHIFT_WIDTH_(shift) * _INDEX_BITS_(index, shift), _SHIFT_WIDTH_(shift)))
#define SET_BITS(value, index, shift) \
    _SHIFTL(value, _SHIFT_WIDTH_(shift) * _INDEX_BITS_(index, shift), _SHIFT_WIDTH_(shift))
#define SET_VALUE(arrindex, index, value, shift) \
    UNSET_BITS(arrindex, index, shift) | SET_BITS(value, index, shift)

#define PACKED_SET_2(arr, index, subindex, value)   PACKED_INDEX_2(arr, index, subindex) = (value)
#define PACKED_SET_4(arr, index, value)             PACKED_INDEX_4(arr, index) = (value)
#define PACKED_SET_8(arr, index, value)             PACKED_INDEX_8(arr, index) = SET_VALUE(PACKED_INDEX_8(arr, index), index, value, 3)
#define PACKED_SET_16(arr, index, value)            PACKED_INDEX_16(arr, index) = SET_VALUE(PACKED_INDEX_16(arr, index), index, value, 4)

#endif
