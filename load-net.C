
# include <tclap/CmdLine.h>
# include <net.H>

bool verbose = false;

using namespace TCLAP;

void process_comand_line(int argc, char *argv[])
{
  CmdLine cmd("load-data", ' ', "0.0");

  SwitchArg verbose("v", "verbose", "verbose mode", false);
  cmd.add(verbose);

  SwitchArg test("t", "test", "test for consistency", true);
  cmd.add(test);

  ValueArg<string> meta("m", "meta-data", "nombre archivo de archivo data", 
			true, "data.txt", 
			"nombre de archivo dónde se escribirán los datos");
  cmd.add(meta);

  cmd.parse(argc, argv);

  ::verbose = verbose.getValue();

  ifstream in(meta.getValue());
  if (in.fail())
    throw domain_error(fmt("No puedo abrir %s", meta.getValue().c_str()));

  MetaMapa mapa(in);
  
  if (test.getValue())
    mapa.autotest();

  mapa.save(cout);
}


int main(int argc, char *argv[])
{
  process_comand_line(argc, argv);
}
