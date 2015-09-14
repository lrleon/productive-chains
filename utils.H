
inline size_t levenshtein(const string &s1, const string &s2)
{
  size_t N1 = s1.size();
  size_t N2 = s2.size();
  size_t i, j;
  vector<size_t> T(N2+1);

   for (i = 0; i <= N2; i++)
      T[i] = i;

   for (i = 0; i < N1; i++) 
   {
     T[0] = i+1;
     size_t corner = i;
     for (j = 0; j < N2; j++)
       {
         size_t upper = T[j+1];
         if (s1[i] == s2[j])
	   T[j+1] = corner;
         else
	   T[j+1] = min(T[j], min(upper, corner)) + 1;
         corner = upper;
       }
   }
   return T[N2];
}

inline string only_alpha(const string & str)
{
  string ret;
  for (size_t i = 0, n = str.size(); i < n; ++i)
    {
      auto c = str[i];
      switch (c)
	{
	case '0'...'9':
	case 'a'...'z': ret.push_back(c); break;
	case 'A'...'Z': ret.push_back(tolower(c)); break;
	default: break;
	}
      }
  return ret;
}
