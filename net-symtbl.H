
# ifndef NET_SYMTBL_H
# define NET_SYMTBL_H

# include <tpl_dynSetHash.H>

struct Table : private DynSetHash<string>
{
  char * addstring(char * str)
  {
    const char * s = search_or_insert(string(str))->c_str();
    assert(s != str);
    return const_cast<char*>(s);
  }

  char * operator () (char * str) const noexcept
  {
    auto ptr = search(string(str));
    return ptr ? const_cast<char*>(ptr->c_str()) : nullptr;
  }
};

using StringTable = Table;
using IdTable = Table;

struct SymbolTable : private DynMapHash<string, Varname*>
{
  Varname * operator () (char * str) const 
  {
    auto p = search(string(str));
    if (p == nullptr)
      return nullptr;
    return p->second;
  }

  Varname * operator () (const string & str) const
  {
    return (*this)((char*) str.c_str());
  }

  void addvar(const string & name, Varname * var)
  {
    assert(search(name) == nullptr);
    insert(name, var);
  }

  ~SymbolTable()
  {
    this->for_each([] (auto p) { delete p.second; });
  }
};

extern StringTable string_table;
extern IdTable id_table;

extern SymbolTable var_tbl;

# endif // NET_SYMTBL_H
