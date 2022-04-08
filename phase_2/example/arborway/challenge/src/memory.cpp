#include "memory.hpp"

Memory::Memory( )
{
    this->pagebits = 12;

    return;
}

void Memory::walkTheBlock( )
{
    MemBlock *mb = NULL;
    uint64_t currentPage = 0;
    uint64_t nextPage = 0;

    std::vector<uint64_t> keys;

    for ( auto const& element : page_to_memblock ) {
        keys.push_back(element.first);
    }

    std::sort ( keys.begin(), keys.end());

    for ( auto it = keys.begin(); it != keys.end(); ++it) {
        currentPage = *it;

        if ( currentPage < nextPage) {
            continue;
        }

        mb = page_to_memblock[currentPage];

        assert( mb != NULL );

        std::cout << "0x" << mb->start << "-0x" << mb->end << " " << mb->size << " " << mb->perms << std::endl;

        nextPage = page(mb->end);
    }

    return;
}

int Memory::write_l( void *addr, const char *data, uint64_t length)
{
	MemBlock *m = NULL;
	uint64_t from_index = 0;
	uint64_t to_index = 0;
	uint64_t start = reinterpret_cast<uint64_t>(addr);
	uint64_t end = start + length;

	if ( addr == NULL || data == NULL ) {
		return -1;
	}

    if ( isWriteable( start, length) == false) {
        throw MemoryException();
    }

	while ( start < end ) {
		m = this->getBlock( start );

		if ( m == NULL ) {
			return -1;
		}

		to_index = start - m->start;

		while ( start < end && start < m->end ) {
            
			m->data[to_index] = data[from_index];

			to_index++;
			from_index++;
			start++;
		}
	}

	return 0;
}

/// TODO: see if I can just use write_l
int Memory::writeymm( void *addr, uint256_t value, uint64_t length)
{
    MemBlock *m = NULL;
    uint64_t to_index = 0;
    uint64_t start = reinterpret_cast<uint64_t>(addr);
    uint64_t end = start + length;
    uint8_t byte;
    uint256_t tempvalue = value;

    if ( addr == NULL ) {
        return -1;
    }

    if ( isWriteable( start, length) == false) {
        throw MemoryException();
    }

    while ( start < end ) {
        m = this->getBlock( start );

        if ( m == NULL ) {
            return -1;
        }

        to_index = start - m->start;

        while ( start < end && start < m->end ) {
            
            byte = value & 0xff;
            value >>= 8;

            m->data[to_index] = byte;

            to_index++;
            start++;
        }
    }

    return 0;
}

uint64_t Memory::getPerms( uint64_t addr )
{
    MemBlock *m = NULL;
    // TODO: make sure it is a mapped page

    m = this->getBlock( reinterpret_cast<uint64_t>(addr));

    if ( m ) {
        return m->perms;
    }

    // TODO throw c++ error
    return 0;
}

int Memory::readstring( uint64_t addr, std::string &out)
{
    uint64_t start = addr;
    uint64_t index = 0;
    uint8_t b;
    MemBlock *mb = NULL;
    std::string t = "";

    do { 
        mb = getBlock(start);

        if ( mb == NULL ) {
            return -1;
        }

        // Make sure that the block is readable
        if ( (mb->perms & PROT_READ) == 0 ) {
            return -1;
        }

        index = start - mb->start;

        while ( start < mb->end ) {
            b = mb->data[index];

            start++;
            index++;

            if ( b == 0x00) {
                out = t;
                return 0;
            }

            t += b;
        }

    } while (b != 0x00);

    out = t;
    return 0;
}

bool Memory::isWriteable( uint64_t addr, uint64_t size )
{
    MemBlock *mb = NULL;
    uint64_t start = addr;
    uint64_t end = addr + size;

    while ( start < end ) {
        mb = getBlock( start );

        if ( mb == NULL ) {
            return false;
        }

        if ( (mb->perms & PROT_WRITE) == 0 ) {
            return false;
        }

        start = mb->end;
    }

    return true;
}

bool Memory::isReadable( uint64_t addr, uint64_t size )
{
    MemBlock *mb = NULL;
    uint64_t start = addr;
    uint64_t end = addr + size;

    while ( start < end ) {
        mb = getBlock( start );

        if ( mb == NULL ) {
            return false;
        }

        if ( (mb->perms & PROT_READ) == 0 ) {
            return false;
        }

        start = mb->end;
    }

    return true;
}


// Rounds down to the nearest page
uint64_t Memory::floor( uint64_t addr )
{
    // The default pagebits is 12 so lets say we have the addres 0x41411234
    // 1 << 12 = 0x1000
    // subtracting 1 gives you 0xfff
    // Taking the twos complement on a 64 bit value gives you 0xfffffffffffff000
    // The resulting masking operation returns 0x41411234 & 0xfffffffffffff000 giving 0x41411000
    return addr & ~((1 << this->pagebits) - 1);
}

uint64_t Memory::page( uint64_t addr)
{
    return addr >> this->pagebits;
}

MemBlock *Memory::getBlock( uint64_t addr)
{
    auto it = this->page_to_memblock.find( this->page(addr) );

    if ( it == this->page_to_memblock.end() ) {
	    return NULL;
    }

    return it->second;
}

// Rounds up to the nearest valid address in this page
// This can also be used to calculate the 
uint64_t Memory::ceil( uint64_t addr )
{
    /// Take the given address and add a value up to the next page
    /// Then take the floor of this minus 1 to get the max
    // This works even if it is the last page.
    // 0x41411234 + 0x1000 = 0x41412234
    // The floor of this is 0x41412000
    return this->floor( addr + ( 1 << this->pagebits) );
}

int Memory::putchar( void *addr, uint8_t c)
{
    MemBlock *m = NULL;
    uint64_t index;
    uint64_t taddr;

    if ( addr == NULL ) {
        return -1;
    }

    taddr = reinterpret_cast<uint64_t>(addr);

    m = this->getBlock( taddr );

    if ( !m ) {
        return -1;
    }

    if ( isWriteable( taddr, 1) == false) {
        throw MemoryException();
    }

    index = taddr - m->start;

    m->data[index] = c;

    return 0;
}

int Memory::read_l( void *addr, char *dest, uint64_t size)
{
    MemBlock *m = NULL;
    uint64_t int_addr = 0;
    uint64_t start = 0;
    uint64_t end =0 ;
    uint64_t index = 0;
    uint64_t byte_index = 0;

    if ( addr == NULL || dest == NULL) {
        return -1;
    }

    int_addr = reinterpret_cast<uint64_t>(addr);

    if ( isReadable( int_addr, size) == false) {
        throw MemoryException();
    }

    start = int_addr;
    end = int_addr + size;

    while ( start < end ) {
        m = getBlock( int_addr );

        if ( m == NULL ) {
            return -1;
        }

        index = start - m->start;

        while ( start < end && start < m->end ) {
            dest[byte_index] = m->data[index];

            byte_index++;
            start++;
            index++;
        }

    }

    return 0;
}

int Memory::readint( void *addr, uint64_t *dest, uint64_t size)
{
    uint64_t value = 0;
    uint8_t *tvalue = (uint8_t*)&value;

    MemBlock *m = NULL;
    uint64_t int_addr = 0;
    uint64_t start = 0;
    uint64_t end =0 ;
    uint64_t index = 0;
    uint64_t byte_index = 0;

    if ( addr == NULL || dest == NULL) {
        return -1;
    }

    if ( size != 1 && size != 2 && size != 4 && size != 8 ) {
        return -1;
    }

    int_addr = reinterpret_cast<uint64_t>(addr);

    start = int_addr;
    end = int_addr + size;

    if ( isReadable( int_addr, size) == false) {
        throw MemoryException();
    }

    while ( start < end ) {
        m = getBlock( int_addr );

        if ( m == NULL ) {
            return -1;
        }

        index = start - m->start;

        while ( start < end && start < m->end ) {
            tvalue[byte_index] = m->data[index];

            byte_index++;
            start++;
            index++;
        }

    }

    *dest = value;
    return 0;
}


int Memory::readint( void *addr, uint256_t *dest, uint64_t size)
{
    uint256_t value = 0;
    uint8_t byte;

    MemBlock *m = NULL;
    uint64_t int_addr = 0;
    uint64_t start = 0;
    uint64_t end =0 ;
    uint64_t index = 0;
    uint64_t byte_index = 0;

    if ( addr == NULL || dest == NULL) {
        std::cout << "[ERROR] 256 dest NULL or addr NULL" << std::endl;
        return -1;
    }

    if ( size != 1 && size != 2 && size != 4 && size != 8 && size != 16 && size != 32 ) {
        std::cout << "[ERROR] Invalid size: " << (int) size << std::endl;
        return -1;
    }

    int_addr = reinterpret_cast<uint64_t>(addr);

    start = int_addr;
    end = int_addr + size;

    if ( isReadable( int_addr, size) == false) {
        throw MemoryException();
    }
    
    while ( start < end ) {
        m = getBlock( int_addr );

        if ( m == NULL ) {
            return -1;
        }

        index = start - m->start;

        while ( start < end && start < m->end ) {
            byte = m->data[index];

            value |= uint256_t(byte) << (byte_index * 8);

            byte_index++;
            start++;
            index++;
        }

    }

    *dest = value;

    return 0;
}

// This loops through the allocated space to find the first free block
//  following the start address
uint64_t Memory::find_empty_block( uint64_t start, uint64_t size)
{
    MemBlock *m;

    /// I won't allocate more than 1000 pages
    /// TODO actually throw an error
    if ( size > 1024 * 1024) {
        std::cout << "[ERROR] invalid size" << std::endl;
        exit(0);
    }

    if ( start + size < start ) {
        start = 1 << this->pagebits;
    }

    for ( uint64_t i = this->page( start); i <= this->page( this->ceil( start + size - 1) ); i++ ) {
        if ( this->page_to_memblock.find(i) != this->page_to_memblock.end() ) {
            m = this->page_to_memblock[i];

            /// If this page is already allocated then jump to the page follwoing the end of this mapping
            ///     and continue the search.

            return this->find_empty_block( m->end, size );
        }
    }

    return start;
}

int Memory::mprotect_l( void *addr, uint64_t size, uint64_t perms)
{
    MemBlock *left = NULL;
    MemBlock *middle = NULL;
    MemBlock *right = NULL;

    uint64_t start;
    uint64_t end;

    uint64_t result;

    if ( addr == NULL ) {
        return -1;
    }

    // TODO make sure the perms are legit

    // I need to figure out how many blocks are affected.
    // These blocks then need to be split up

    // get the starting address
    start = reinterpret_cast<uint64_t>(addr);
    end = this->ceil(start + size - 1);

    /// TODO check address validity
    if ( end < start ) {
        return -1;
    }

    while ( start < end ) {
        left = this->getBlock( start );

        // There are 3 possibilities. 
        //  This block is entirely encompased by the mprotect request
        //  The request starts and ends in the block
        //  The request starts in the block but ends later

        if ( left == NULL ) {
            return -1;
        }

        if ( left->start == start && left->end == end) {
            left->perms = perms;

            start = left->end;

            result = mprotect( (void*)left->start, left->size, perms);

            if (  result != 0 ) {

                return result;
            }
        } else if ( left->start < start && end < left->end ) {

            /// change middle block
            middle = new MemBlock( start, end - start, perms, this->pagebits);

            if ( middle == NULL ) {
                exit(0);
            }

            right = new MemBlock( end, left->end - end, left->perms, this->pagebits );

            if ( right == NULL ) {
                exit(0);
            }

            left->end = start;
            left->size = left->end - left->start;

            middle->data = (char*)middle->start;

            right->data = (char*)right->start;

            /// fixup all the page mappings
            for ( uint64_t i = this->page( middle->start ); i < this->page( this->ceil( middle->end ) - 1); i++ ) {

                this->page_to_memblock[i] = middle;
            }

            for ( uint64_t i = this->page( right->start ); i < this->page( this->ceil( right->end ) - 1); i++ ) {

                this->page_to_memblock[i] = right;
            }

            result = mprotect( (void*)middle->start, middle->size, perms);

            if (  result != 0 ) {
                std::cout << "[ERROR] Failed to mprotect @" << std::hex << left->start << std::endl;

                return result;
            }

            start = right->end;
        } else if ( left->start < start && left->end <= end ) {

            /// right side
            right = new MemBlock( start, left->end - start, perms, this->pagebits);

            if ( right == NULL ) {
                // TODO better error
                std::cout << "[FAIL] memory allocation failed" << std::endl;
                exit(0);
            }

            left->end = start;
            left->size = left->end - left->start;

            right->data = (char*)right->start;

            /// fixup all the page mappings
            for ( uint64_t i = this->page( right->start ); i < this->page( this->ceil( right->end ) - 1); i++ ) {
                this->page_to_memblock[i] = right;
            }

            result = mprotect( (void*)right->start, right->size, perms);

            if (  result != 0 ) {
                return result;
            }

            start = right->end;
        }  else if ( left->start == start && end < left->end ) {
            //std::cout << "[MPROTECT] left section" << std::endl;

            /// mprotect the left side
            right = new MemBlock( end, left->end - end, left->perms, this->pagebits);

            if ( right == NULL ) {
                // TODO better error
                //std::cout << "[FAIL] memory allocation failed" << std::endl;
                exit(0);
            }

            //std::cout << "LEFT END: " << left->end << std::endl;
            //std::cout << "RIGHT Start: " << right->start << std::endl;
            //std::cout << "RIGHT END: " << right->end << std::endl;

            left->end = end;
            left->size = left->end - left->start;
            left->perms = perms;

            //std::cout << "4 SETTING data to: 0x" << right->start << std::endl;
            right->data = (char*)right->start;

            //std::cout << "RSP: " << this->page( right->start ) << " REP: " << this->page( this->ceil( right->end ) - 1) << std::endl;
            for ( uint64_t i = this->page( right->start ); i < this->page( this->ceil( right->end ) - 1); i++ ) {
                //std::cout << "mprotect marking page: " << i << std::endl;
                this->page_to_memblock[i] = right;
            }

            result = mprotect( (void*)left->start, left->size, perms);

            if ( result != 0 ) {
                //std::cout << "[ERROR] Failed to mprotect @" << std::hex << left->start << std::endl;

                return result;
            }

            //std::cout << "Split block @" << std::hex << left->start << " into: " << left->start << " size: " << left->size << " @" << right->start << " size: " << right->size << std::endl;
            //std::cout << "Change perms to memory @" << std::hex << left->start << " size:" << left->size << " newperms: " << left->perms << std::endl;

            start = right->end;

        } else {
            exit(0);
        }
    }

    return 0;
}

int Memory::munmap_l( void *addr, uint64_t size )
{
    MemBlock *left = NULL;
    MemBlock *right = NULL;

    uint64_t start;
    uint64_t end;

    if ( addr == NULL ) {
        return -1;
    }

    // I need to figure out how many blocks are affected.
    // These blocks then need to be split up
    /// Just copied from mprotect

    // get the starting address
    start = reinterpret_cast<uint64_t>(addr);
    end = this->ceil( start + size - 1);

    /// TODO check address validity
    if ( end < start ) {
        return -1;
    }

    //std::cout << "[MUNMAP] Start: 0x" << start << " size: 0x" << size << " end: 0x" << end << std::endl;

    while ( start < end ) {
        left = this->getBlock( start );

        //std::cout << "[MUNMAP] Breaking up block containing: 0x" << start << std::endl;

        /// Per the man page: It is not an error if the  indicated  range  does  not  contain  any mapped pages.
        if ( left == NULL ) {
            //std::cout << "[MUNMAP] Didn't find block starting at 0x" << start << std::endl;
            start = this->ceil( start );

            continue;
        }

        //std::cout << "[MUNMAP] Block start: 0x" << left->start << " end: 0x" << left->end << " size: 0x" << left->size << std::endl;

        // There are 3 possibilities. 
        //  This block is entirely encompased by the munmap request
        //  The request starts and ends in the block
        //  The request starts in the block but ends later

        if ( left->start == start && left->end <= end) {
            /// Unmap the whole block

            //std::cout << "[MUNMAP] Unmapping the entire block" << std::endl;

            if ( munmap( (void*)left->start, left->size ) != 0 ) {
                //std::cout << "[ERROR] Failed to unmap: 0x" << std::hex << left->start << std::endl;

                return -1;
            }

            start = left->end;

            /// Remove page mappings
            for ( uint64_t i = this->page( left->start ); i < this->page( this->ceil( left->end ) - 1); i++ ) {
                //std::cout << "\tRemoving page mapping: " << (int)i << std::endl;

                auto it = this->page_to_memblock.find(i);

                if ( it != this->page_to_memblock.end() ) {
                    this->page_to_memblock.erase( it );
                }
            }

            delete left;

            //std::cout << "Unmapping entire block @" << std::hex << start << std::endl;
        } else if ( left->start < start && end < left->end ) {
            /// Unmap the middle
            right = new MemBlock( end, left->end - end, left->perms, this->pagebits );

            if ( right == NULL ) {
                // TODO better error
                //std::cout << "[FAIL] memory allocation failed" << std::endl;
                exit(0);
            }

            left->end = start;
            left->size = left->end - left->start;

            //std::cout << "5 SETTING data to: 0x" << right->start << std::endl;
            right->data = (char*)right->start;

            // Unmapping the middle
            if ( munmap( (void*)start, end - start) != 0 ) {
                //std::cout << "[ERROR] Failed to unmap: 0x" << std::hex << left->start << std::endl;

                return -1;
            }

            /// fixup all the page mappings
            for ( uint64_t i = this->page( start ); i < this->page( this->ceil( end ) - 1); i++ ) {

                auto it = this->page_to_memblock.find(i);

                if ( it != this->page_to_memblock.end() ) {
                    this->page_to_memblock.erase( it );
                }
            }

            for ( uint64_t i = this->page( right->start ); i < this->page( this->ceil( right->end ) - 1); i++ ) {

                this->page_to_memblock[i] = right;
            }

            ///std::cout << "Split block @" << std::hex << left->start << "into: " << left->start << " size: " << left->size << " @" << right->start << " size: " << right->size << std::endl;

            start = right->end;
        } else if ( left->start < start && left->end <= end ) {
            /// Umap the right side
            //std::cout << "[MUNMAP] taking out the right side" << std::endl;

            if ( munmap( (void*)start, left->end - start) != 0 ) {
                //std::cout << "[ERROR] Failed to unmap: 0x" << std::hex << left->start << std::endl;

                return -1;
            }

            uint64_t next_start = left->end;

            left->end = start;

            left->size = left->end - left->start;

            /// fixup all the page mappings
            for ( uint64_t i = this->page( start ); i < this->page( this->ceil( next_start ) - 1); i++ ) {
                auto it = this->page_to_memblock.find(i);

                if ( it != this->page_to_memblock.end() ) {
                    this->page_to_memblock.erase( it );
                }
            }

            //std::cout << "Split block @" << std::hex << left->start << " into: " << left->start << " size: " << left->size << " end: 0x" << left->end << std::endl;

            start = next_start;
        } else if ( left->start == start && left->end > end ) {
            /// Unmap the left side

            left->start = end;
            left->size = left->end - end;
            left->data = (char*)left->start;

            if ( munmap( (void*)start, end - start) != 0 ) {
                std::cout << "[ERROR] Failed to unmap: 0x" << std::hex << left->start << std::endl;

                return -1;
            }

            /// fixup all the page mappings
            for ( uint64_t i = this->page( start ); i < this->page( this->ceil( end ) - 1); i++ ) {
                auto it = this->page_to_memblock.find(i);

                if ( it != this->page_to_memblock.end() ) {
                    this->page_to_memblock.erase( it );
                }
            }

            start = end;

        } else {
            exit(0);
        }
    }

    return 1;
}



void *Memory::mmap_l( void *addr, uint64_t size, uint64_t perms, uint32_t flags, uint64_t fd, uint64_t offset )
{
    void *r = NULL;
    MemBlock *m = NULL;

    addr = (void *)this->floor( reinterpret_cast<uint64_t>(addr) );

    // The - 1 ensures that we don't allocate an extra page
    size = this->ceil( size - 1 );

    addr = (void *)this->find_empty_block( reinterpret_cast<uint64_t>(addr), size );

    r = mmap( addr, size, perms, flags, fd, offset);

    if ( r == MAP_FAILED) {
        std::cout << "mmap fail: " << strerror( errno ) << std::endl;
        return NULL;
    }

    //memset( r, 0, size );

    m = new MemBlock(reinterpret_cast<uint64_t>(r), size, perms, this->pagebits);

    if ( m == NULL ) {
        munmap( r, size );

        r = NULL;

        return r;
    }

    /// Mark all the pages and save the memory block.
    for ( uint64_t i = this->page( m->start ); i < this->page( this->ceil( m->end ) - 1 ); i++ ) {

        this->page_to_memblock[i] = m;
    }

    //std::cout << "m->start " << m->start << std::endl;
    //std::cout << "6 SETTING data to 0x" << r << std::endl;
    m->data = (char*)m->start;

    return r;
}

/// Read a portion of a file into memoty and return a pointer to it.
void *Memory::mmapFile( void *addr, uint64_t size, uint32_t flags, uint32_t perms, uint64_t offset, FILE *fp )
{
    void *r = NULL;
    uint64_t read_size = size;

    MemBlock *m = NULL;

    if ( !fp ) {
        return r;
    }

    addr = (void *)this->floor( reinterpret_cast<uint64_t>(addr) );

    size = this->ceil( size - 1 );

    addr = (void *)this->find_empty_block( reinterpret_cast<uint64_t>(addr), size );

    r = mmap( addr, size, perms | PROT_WRITE, flags, 0, 0);

    if ( r == MAP_FAILED) {
        std::cout << "mmap fail: " << strerror( errno ) << std::endl;
        return NULL;
    }

    memset( r, 0, size );

    if ( fseek(fp, offset, SEEK_SET) != 0 ) {
        munmap( r, size );

        r = NULL;

        return r;
    }

    fread( r, 1, read_size, fp);

    /// uint64_t start, uint64_t size, uint64_t perms, uint64_t pagebits
    /// Set it to allocated and mark all the pages
    m = new MemBlock(reinterpret_cast<uint64_t>(r), size, perms, this->pagebits);

    if ( m == NULL ) {
        munmap( r, size );

        r = NULL;

        return r;
    }

    /// Mark all the pages and save the memory block.
    for ( uint64_t i = this->page( m->start ); i < this->page( this->ceil( m->end ) - 1); i++ ) {

        this->page_to_memblock[i] = m;
    }

    if ( ! ( perms & PROT_WRITE) ) {
        //int mprotect(void *addr, size_t len, int prot);
        /// TODO FIX THIS
        //mprotect( r, size, perms);
    }

    m->data = (char*)m->start;

    return r;
}
