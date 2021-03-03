#ifndef SYSTEMEXCEPTION_H
#define SYSTEMEXCEPTION_H

#include <exception>
#include <string>

/**
 * Reports system error.
 */
class SystemException: public std::exception
{
public:

    /**
     * Constructs an exception.
     * 
     * @param message The message associated with the exception.
     */
    SystemException(std::string &message);

    /**
     * Constructs an exception.
     * 
     * @param message The message associated with the exception.
     */
    SystemException(const char *message);    

    /**
     * Returns the message associated with this exception.
     */
    virtual const char* what() const throw();

    /**
     * Allow for easy print of the exception.
     * 
     * @param out The output stream
     * @param e The exception
     * @return the output stream
     */
    friend std::ostream &operator<<(std::ostream &out, const SystemException &e);

private:

    std::string data;
};

#endif