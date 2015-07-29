
# include <tpl_tree_node.H>

using namespace std;


struct Command : public Tree_Node_Vtl<Empty_Class>
{
  string error_msg;
  bool error = false;

  void set_error(string msg) 
  {
    error_msg = std::move(msg);
    error = true;
  }

  bool was_executed() const noexcept { return error; }
  bool on_error() const noexcept { return was_executed(); }

  virtual bool execute() = 0;
  virtual ostream & print(ostream&) = 0;
};

struct CommandList : public DynList<Command*>
{
  
};

struct Load : public Command
{
  string file_name;

  Load(const string & name) noexcept : file_name(name) {}

  bool execute() { return true; }

  ostream & print(ostream & out) { return out << "LOAD " << file_name;
};

struct Save : public Command
{
  string file_name;

  Save(const string & name) : file_name(name) {}

  bool execute() { return true; }

  ostream & print(ostream & out) { return out << "SAVE " << file_name;
};

struct Varname : public Command
{
  string name;

  Varname(const string & nom) : name(nom) {}

  bool execute() { return true; }

  ostream & print(ostream & out) { return out << "Var = " << file_name;
};

