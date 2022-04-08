#ifndef __EXECPTIONS_HPP__
#define __EXECPTIONS_HPP__

#include <iostream>
#include <exception>

class InvalidIdentifierException : public std::exception
{
public:
    std::string id;

    InvalidIdentifierException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "Unknown identifier encountered";
    }

};

class GenericException : public std::exception
{
public:
    std::string id;

    GenericException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "exception";
    }

};

class DivByZeroException : public std::exception
{
public:
    std::string id;

    DivByZeroException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "divide by zero";
    }

};

class InvalidOperationException : public std::exception
{
public:
    std::string id;

    InvalidOperationException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "invalid operation encountered";
    }

};

class UnknownFunctionException : public std::exception
{
public:
    std::string id;

    UnknownFunctionException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "Unknown function encountered";
    }

};

class InvalidTypeException : public std::exception
{
public:
    std::string id;

    InvalidTypeException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "Unknown type encountered";
    }

};

class InvalidFunctionException : public std::exception
{
public:
    std::string id;

    InvalidFunctionException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "Unknown function encountered";
    }

};

class InvalidFunctionArgCountException : public std::exception
{
public:
    std::string id;

    InvalidFunctionArgCountException( std::string id ) : id(id) { }

    const char * what () const throw ()
    {
        return "invalid function argument count";
    }

};

class InvalidFunctionArgTypeException : public std::exception
{
public:
    std::string expected;
    std::string received;

    InvalidFunctionArgTypeException( std::string expected, std::string received ) : expected(expected), received(received) { }

    const char * what () const throw ()
    {
        return "invalid function argument type";
    }

};

#endif