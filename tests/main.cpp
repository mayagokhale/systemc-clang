// #define CATCH_CONFIG_MAIN
// #include <catch.hpp>
//
//

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

std::string data_file; // Some user variable you want to be able to set
int main(int argc, char** argv) {
    doctest::Context context;

    // !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

    // defaults
    context.setOption("abort-after", 5);              // stop test execution after 5 failed assertions
    context.setOption("order-by", "name");            // sort the test cases by their name

    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail

    int res = context.run(); // run

    if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        return res;          // propagate the result of the tests
    
    int client_stuff_return_code = 0;
    // your program - if the testing framework is integrated in your production code
    
    return res + client_stuff_return_code; // the result from doctest is propagated here as well
}


/*
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <string>

std::string data_file; // Some user variable you want to be able to set
int main( int argc, char* argv[] )
{
  Catch::Session session; // There must be exactly one instance
  
  
  // Build a new parser on top of Catch's
  using namespace Catch::clara;
  auto cli 
    = session.cli() // Get Catch's composite command line parser
    | Opt( data_file, "data-file" ) // bind variable to a new option, with a hint string
        ["-g"]["--data-file"]    // the option names it will respond to
        ("input data file");        // description string for the help output
        
  // Now pass the new composite back to Catch so it uses that
  session.cli( cli ); 
  
  // Let Catch (using Clara) parse the command line
  int returnCode = session.applyCommandLine( argc, argv );
  if( returnCode != 0 ) // Indicates a command line error
      return returnCode;

  // if set on the command line then 'height' is now set at this point
  if( data_file!= "" )
      std::cout << "data_file: " << data_file << std::endl;

  return session.run();
}
*/
