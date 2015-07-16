
# include <parse-csv.H>

int main()
{
  std::ifstream in("test.csv");
  if (in.fail()) 
    return (cout << "File not found" << endl) && 0;
  
  while (in.good())
    {
      auto row = csv_read_row(in, ',');
      row.for_each([] (auto s) { cout << s << "\t"; });
      cout << endl;
    }
  in.close();
 
  std::string line;
  in.open("test.csv");
  while(getline(in, line)  && in.good())
    {
      auto row = csv_read_row(line, ',');
      row.for_each([] (auto s) { cout << "[" << s << "]" << "\t"; });
      cout << endl;
    }
  in.close();

  return 0;
}
