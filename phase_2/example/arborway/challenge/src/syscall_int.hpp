#ifndef __SYSEXC_HPP__
#define __SYSEXC_HPP__

#include <iostream>
#include <exception>

class SycallException : public std::exception
{
public:
    uint64_t error_number;

    SycallException( uint64_t err_num ) {
        error_number = err_num;
    }

    SycallException( ) {
        error_number = 0;
    }

    const char * what () const throw ()
    {
        return "syscall interrupt";
    }

};

class MemoryException : public std::exception
{
public:
    uint64_t error_number;

    MemoryException( uint64_t err_num ) {
        error_number = err_num;
    }

    MemoryException( ) {
        error_number = 0;
    }

    const char * what () const throw ()
    {
        return "memory exception";
    }

};

class InstructionException : public std::exception
{
public:
    uint64_t error_number;

    InstructionException( uint64_t err_num ) {
        error_number = err_num;
    }

    InstructionException( ) {
        error_number = 0;
    }

    const char * what () const throw ()
    {
        return "instruction exception";
    }

};

class FloatingPointException : public std::exception
{
public:
    uint64_t error_number;

    FloatingPointException( uint64_t err_num ) {
        error_number = err_num;
    }

    FloatingPointException( ) {
        error_number = 0;
    }

    const char * what () const throw ()
    {
        return "FloatingPoint exception";
    }

};

#endif