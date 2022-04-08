#include "register.hpp"

reg::reg( )
{
    return;
}

reg::reg( std::string name, uint64_t bits)
{
    this->name = name;
    this->bits = bits;
}

void reg::set( uint64_t nv, uint64_t loc )
{
    uint64_t mask_old;
    uint64_t mask_new;

    switch ( loc ) {
        case LOWBYTE:
            mask_old = 0xffffffffffffff00;
            mask_new = 0xff;

            this->val = (val & mask_old) | (nv & mask_new);
            break;
        case HIGHBYTE:
            mask_old = 0xffffffffffff00ff;
            mask_new = 0xff;

            this->val = (val & mask_old) | ( (nv & mask_new) << 8);
            break;
        case WORD:
            mask_old = 0xffffffffffff0000;
            mask_new = 0xffff;

            this->val = (val & mask_old) | (nv & mask_new);
            break;
        case DWORD:
            mask_old = 0xffffffff00000000;
            mask_new = 0xffffffff;

            this->val = nv & mask_new;
            break;
        case QWORD:
            this->val = nv;
            break;
        default:
            std::cout << "[ERROR] Invalid type: " << loc << std::endl;
            exit(0);
            break;
    };

    return;
}

void reg::set( uint256_t nv, uint64_t loc )
{
    switch ( loc ) {
        case DQWORD:
            nv = (nv << 128) >> 128;
            
            this->ymm = ( (ymm >> 128) << 128) | nv;
            break;
        case QQWORD:
            this->ymm = nv;
            break;
        default:
            std::cout << "[ERROR] Invalid type: " << loc << std::endl;
            exit(0);
            break;
    };

    return;
}

uint64_t reg::get(uint64_t loc )
{
    uint64_t mask;
    uint64_t tv;

    switch ( loc ) {
        case LOWBYTE:
            mask = 0xff;

            tv =  this->val & mask;
            break;
        case HIGHBYTE:
            mask = 0xff;

            tv = (this->val >> 8) & mask ;
            break;
        case WORD:
            mask = 0xffff;

            tv = this->val & mask;
            break;
        case DWORD:
            mask = 0xffffffff;

            tv = this->val & mask;
            break;
        case QWORD:
            tv = this->val;
            break;
        default:
            std::cout << "[ERROR] Invalid 64-bit loc type: " << loc << std::endl;
            exit(0);
            break;
    };

    return tv;
}

uint256_t reg::get(uint256_t loc )
{
    uint256_t mask = (uint256_t(0xffffffffffffffff) << 64) | uint256_t(0xffffffffffffffff);
    uint256_t xmm;

    switch ( uint64_t(loc) ) {
        case DQWORD:
            xmm = this->ymm & mask;
            break;
        case QQWORD:
            xmm = this->ymm;
            break;
        default:
            std::cout << "[ERROR] Invalid type: " << loc << std::endl;
            exit(0);
            break;
    };

    return xmm;
}