#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <utility>
#include <cassert>

struct BufferedFileIterator{
    BufferedFileIterator(uint8_t const* const& data, int pos = 0):
        data(data), _position(pos)
    {}

    BufferedFileIterator& operator--(){
         return (*this) -= 1;
    }

    BufferedFileIterator& operator++(){
        return (*this) += 1;
    }

    BufferedFileIterator& operator-=(int inc){
        _position -= inc;
        return *this;
    }

    BufferedFileIterator& operator+=(int inc){
        _position += inc;
        return *this;
    }

    BufferedFileIterator operator+(int inc){
        return BufferedFileIterator(data, _position + inc);
    }

    uint8_t operator[](int idx) const {
        return *(data + _position + idx);
    }

    uint8_t const* operator*() const {
        return data + _position;
    }

    int position() const {
        return _position;
    }

    private:
        uint8_t const* data;
        int _position;
};

struct BufferedFile{
public:
    BufferedFile(const char* file_name){
        FILE* program_file = fopen(file_name, "r");
        _data.reserve(8192);
        int c;
        do {
            c = fgetc(program_file);

            // do not insert EOF char
            if (c != EOF){
                _data.push_back(uint8_t(c));
            } else {
                break;
            }
        }   
        while (true);
        fclose(program_file);
    }

    BufferedFileIterator cursor(int pos = 0) const{
        return BufferedFileIterator(data(), pos);
    }

    uint8_t const* data() const { return &_data[0]; }
    int            size() const { return int(_data.size()); }
private:
    std::vector<uint8_t> _data;
};

struct ElfIdentity{
    #define ELF_IDENTITY(ATTR)\
    ATTR(uint32_t, mag     , "%20x", "Constant Signature")\
    ATTR(uint8_t , class   , "%20d", "32/64 Bits")\
    ATTR(uint8_t , data    , "%20d", "Little (LSB=1)/Big Endian (MSB=2)")\
    ATTR(uint8_t , version , "%20d", "ELF version (should be 1)")\
    ATTR(uint8_t , osabi   , "%20x", "System ABI (Linux: 0x03)")\
    ATTR(uint8_t , abi     , "%20d", "EI_ABIVERSION")

    #define ATTR(type, name, fmt, comment)\
        type ei_##name = 0;
        ELF_IDENTITY(ATTR)
    #undef ATTR

    uint8_t pad[7];
};

struct ElfHeader64{
    #define EI_NIDENT 16
    uint8_t e_ident[EI_NIDENT];

    #define ELF_COMPONENTS(ATTR)\
        ATTR(uint16_t, type     , "%20x", "Object File Type")\
        ATTR(uint16_t, machine  , "%20x", "ISA (amd64 = 0x3e, x86 = 0x03)")\
        ATTR(uint32_t, version  , "%20d", "ELF Version (should be 1)")\
        ATTR(uint64_t, entry    , "%20x", "Address where execution starts")\
        ATTR(uint64_t, phoff    , "%20lu", "Program Header Offset")\
        ATTR(uint64_t, shoff    , "%20d", "Section headers Offset")\
        ATTR(uint32_t, flags    , "%20d", "Flags")\
        ATTR(uint16_t, ehsize   , "%20d", "ELF Header size")\
        ATTR(uint16_t, phentize , "%20d", "Size of single program header")\
        ATTR(uint16_t, phnum    , "%20d", "Count of a program headers")\
        ATTR(uint16_t, shentsize, "%20d", "Size of single section header")\
        ATTR(uint16_t, shnum    , "%20d", "Count of section headers")\
        ATTR(uint16_t, shstrndx , "%20d", "index of the names section in the table")
        
    #define ATTR(type, name, fmt, comment)\
        type e_##name = 0;
        ELF_COMPONENTS(ATTR)
    #undef ATTR
}; 

bool is_big_endian(){
    union {
        uint32_t i;
        char c[4];
    } union_int = {0x01020304};

    return union_int.c[0] == 1; 
}

template<typename T>
void swap_endian(T& value){
    uint8_t* val = reinterpret_cast<uint8_t*>(&value);
    int n = sizeof(T) - 1;

    for(int i = 0; i < sizeof(T) / 2; i++){
        std::swap(val[i], val[n - i]);
    }
}

void print(ElfHeader64& header){
    printf("--\nELF identity\n");
    ElfIdentity* ptr_ident = reinterpret_cast<ElfIdentity*>(&header);
    ElfIdentity& ident = *ptr_ident;

    if (!is_big_endian()){
        swap_endian(ident.ei_mag);
    }

#define ATTR(type, name, fmt, comment)\
    printf("%40s: %10s %15s " fmt "\n", comment, #type, "ei_"#name, (ident).ei_##name);
    ELF_IDENTITY(ATTR)
#undef ATTR

    printf("--\n");
#define ATTR(type, name, fmt, comment)\
    printf("%40s: %10s %15s " fmt "\n", comment, #type, "e_"#name, (header).e_##name);
    ELF_COMPONENTS(ATTR)
#undef ATTR

    if (!is_big_endian()){
        swap_endian(ident.ei_mag);
    }   
}

struct ProgramHeaderTable64{
    #define PROGRAM_HEADER_CMPS(ATTR)\
        ATTR(uint32_t, type  , "%20x" , "Segment Type")\
        ATTR(uint32_t, flags , "%20lu", "Segment Flags")\
        ATTR(uint64_t, offset, "%20x" , "Offset where it should be read")\
        ATTR(uint64_t, vaddr , "%20x" , "Virtual address")\
        ATTR(uint64_t, paddr , "%20x" , "Physical address")\
        ATTR(uint64_t, filesz, "%20x" , "Size on file")\
        ATTR(uint64_t, memsz , "%20x" , "Size in memory")\
        ATTR(uint64_t, align , "%20lu", "0 and 1 specify no alignment")

    #define ATTR(type, name, fmt, comment)\
        type p_##name = 0;
        PROGRAM_HEADER_CMPS(ATTR)
    #undef ATTR
};

void print(ProgramHeaderTable64& header){
    #define ATTR(type, name, fmt, comment)\
        printf("%40s: %10s %15s " fmt "\n", comment, #type, "p_"#name, (header).p_##name);
        PROGRAM_HEADER_CMPS(ATTR)
    #undef ATTR
}

enum SectionType{
    #define SH_ENUMS(SH_ENUM)\
        SH_ENUM(SHT_NULL 	     ,   0         )\
        SH_ENUM(SHT_PROGBITS 	 ,   1         )\
        SH_ENUM(SHT_SYMTAB 	     ,   2         )\
        SH_ENUM(SHT_STRTAB 	     ,   3         )\
        SH_ENUM(SHT_RELA 	     ,   4         )\
        SH_ENUM(SHT_HASH 	     ,   5         )\
        SH_ENUM(SHT_DYNAMIC 	 ,   6         )\
        SH_ENUM(SHT_NOTE 	     ,   7         )\
        SH_ENUM(SHT_NOBITS 	     ,   8         )\
        SH_ENUM(SHT_REL 	     ,   9         )\
        SH_ENUM(SHT_SHLIB 	     ,   10        )\
        SH_ENUM(SHT_DYNSYM 	     ,   11        )\
        SH_ENUM(SHT_INIT_ARRAY 	 ,   14        )\
        SH_ENUM(SHT_FINI_ARRAY 	 ,   15        )\
        SH_ENUM(SHT_PREINIT_ARRAY,   16        )\
        SH_ENUM(SHT_GROUP 	     ,   17        )\
        SH_ENUM(SHT_SYMTAB_SHNDX ,	 18        )\
        SH_ENUM(SHT_LOOS 	     ,   0x60000000)\
        SH_ENUM(SHT_GNU_HASH     ,   0x6ffffff6)\
        SH_ENUM(SHT_GNU_verneed  ,   0x6ffffffe)\
        SH_ENUM(SHT_HIOS 	     ,   0x6fffffff)\
        SH_ENUM(SHT_LOPROC 	     ,   0x70000000)\
        SH_ENUM(SHT_HIPROC 	     ,   0x7fffffff)\
        SH_ENUM(SHT_LOUSER 	     ,   0x80000000)\
        SH_ENUM(SHT_HIUSER 	     ,   0xffffffff)
       
    #define SH_ENUM(name, value)\
        name = value,
        SH_ENUMS(SH_ENUM)
    #undef SH_ENUM
};

enum SectionFlags{
    #define SHF_ENUMS(SHF_ENUM)\
        SHF_ENUM(SHF_WRITE 	         ,   0x1       )\
        SHF_ENUM(SHF_ALLOC 	         ,   0x2       )\
        SHF_ENUM(SHF_EXECINSTR 	     ,   0x4       )\
        SHF_ENUM(SHF_MERGE 	         ,   0x10      )\
        SHF_ENUM(SHF_STRINGS 	     ,   0x20      )\
        SHF_ENUM(SHF_INFO_LINK 	     ,   0x40      )\
        SHF_ENUM(SHF_LINK_ORDER 	 ,   0x80      )\
        SHF_ENUM(SHF_OS_NONCONFORMING,   0x100     )\
        SHF_ENUM(SHF_GROUP 	         ,   0x200     )\
        SHF_ENUM(SHF_TLS 	         ,   0x400     )\
        SHF_ENUM(SHF_MASKOS 	     ,   0x0ff00000)\
        SHF_ENUM(SHF_MASKPROC 	     ,   0xf0000000)

    #define SHF_ENUM(name, value)\
        name = value,
        SHF_ENUMS(SHF_ENUM)
    #undef SHF_ENUM
};

SectionType to_section_type(uint32_t sh_type){
    switch(sh_type){
    #define SH_ENUM(name, value)\
        case value: return name;
        SH_ENUMS(SH_ENUM)
    #undef SH_ENUM
    }

    printf("Unknown sh_type %d %x\n", sh_type, sh_type);
    return SHT_NULL;
}

struct SectionHeader64{
    #define SECTION_HEADER_CMPS(ATTR)\
        ATTR(uint32_t, name     , "%20x", "Reference to a name")\
        ATTR(uint32_t, type     , "%20x", "Header type")\
        ATTR(uint64_t, flags    , "%20x", "Section Attributes")\
        ATTR(uint64_t, addr     , "%20x", "Virtual address of the section")\
        ATTR(uint64_t, offset   , "%20x", "Offset of the section")\
        ATTR(uint64_t, size     , "%20x", "Size in bytes in the file image")\
        ATTR(uint32_t, link     , "%20x", "")\
        ATTR(uint32_t, info     , "%20x", "")\
        ATTR(uint64_t, addralign, "%20lu", "required alignment of the section")\
        ATTR(uint64_t, entsize  , "%20lu", "size in bytes, of each entry")

    #define ATTR(type, name, fmt, comment)\
        type sh_##name = 0;
        SECTION_HEADER_CMPS(ATTR)
    #undef ATTR
};

void print(SectionHeader64& header){
    #define ATTR(type, name, fmt, comment)\
        printf("%40s: %10s %15s " fmt "\n", comment, #type, "sh_"#name, (header).sh_##name);
        SECTION_HEADER_CMPS(ATTR)
    #undef ATTR
}

struct Elf64{
    ElfHeader64 header;
    std::vector<ProgramHeaderTable64>   program_headers;
    std::vector<SectionHeader64>        section_headers;
};

struct StringTable{
    StringTable(std::vector<uint8_t> data):
        strings(data)
    {}

    std::string get_string(int pos) const{
        return std::string((*this)[pos]);
    }

    const char* operator[](int pos) const{
        return reinterpret_cast<const char*>(&strings[pos]);
    }

private:
    std::vector<uint8_t> strings;
};


void parse_section(SectionHeader64& header, BufferedFile& file){
    // Parse String Table
    if (to_section_type(header.sh_type) == SHT_STRTAB){
        printf("String Table\n");
        print(header);

        auto cursor = file.cursor(header.sh_offset);
        std::vector<uint8_t> data(*cursor, *(cursor + header.sh_size));
        StringTable table(data);

        // printf("- %s\n", table[1]);
    }
}


int main(){
    printf("is_big_endian %d\n", is_big_endian());

    BufferedFile program_file("simple");
    printf("File size is %d\n", program_file.size());
    Elf64 program;
    
    // Parse ELF64 Header
    {
        uint8_t* ptr_header = reinterpret_cast<uint8_t*>(&program.header);
        memcpy(ptr_header, program_file.data(), sizeof(ElfHeader64));
        print(program.header);
    }
    
    // Parse Program Headers
    {
        program.program_headers = std::vector<ProgramHeaderTable64>(program.header.e_phnum);
        BufferedFileIterator iter = program_file.cursor(program.header.e_phoff);

        for(uint8_t p = 0; p < program.header.e_phnum; ++p){
            uint8_t* ptr_header = reinterpret_cast<uint8_t*>(&program.program_headers[p]);

            memcpy(ptr_header, (*iter), sizeof(ProgramHeaderTable64));
            iter += sizeof(ProgramHeaderTable64);
        }

        printf("Size of Program Header is %lu\n", sizeof(ProgramHeaderTable64));
        for(auto& header: program.program_headers){
            printf("Program header\n");
            print(header);
        }
    }

    // Parse Section Headers
    {
        program.section_headers = std::vector<SectionHeader64>(program.header.e_shnum);
        BufferedFileIterator iter = program_file.cursor(program.header.e_shoff);
        printf("Current pos is %d\n", iter.position());

        for(uint8_t sh = 0; sh < program.header.e_shnum; ++sh){
            uint8_t* ptr_header = reinterpret_cast<uint8_t*>(&program.section_headers[sh]);

            memcpy(ptr_header, (*iter), sizeof(SectionHeader64));
            iter += sizeof(SectionHeader64);

            to_section_type(program.section_headers[sh].sh_type);
        }
        printf("Current pos is %d\n", iter.position());

        printf("Size of Section Header is %lu\n", sizeof(SectionHeader64));
        for(auto& header: program.section_headers){
            if (header.sh_type != 0){
                // printf("Section header\n");
                parse_section(header, program_file);
            }
        }
    }

    return 0;
}

