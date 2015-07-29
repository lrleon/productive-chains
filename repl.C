
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

# include <iostream>
# include <string>

using namespace std;


int main()
{
    char* input;

    string prompt = "> ";

    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);

    for(;;) {
        // Display prompt and read input (NB: input must be freed after use)...
      input = readline(prompt.c_str());

        // Check for EOF.
        if (!input)
            break;

        // Add input to history.
        add_history(input);
	
	cout << input << endl
	     << endl;

        // Free input.
        free(input);
    }
    return 0;
}
