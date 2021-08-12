//#define CATCH_CONFIG_MAIN
//#include <catch.hpp>

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <string>

std::string height; // Some user variable you want to be able to set

int main( int argc, char* argv[] )
{
  Catch::Session session; // There must be exactly one instance
  
  
  // Build a new parser on top of Catch's
  using namespace Catch::clara;
  auto cli 
    = session.cli() // Get Catch's composite command line parser
    | Opt( height, "height" ) // bind variable to a new option, with a hint string
        ["-g"]["--height"]    // the option names it will respond to
        ("how high?");        // description string for the help output
        
  // Now pass the new composite back to Catch so it uses that
  session.cli( cli ); 
  
  // Let Catch (using Clara) parse the command line
  int returnCode = session.applyCommandLine( argc, argv );
  if( returnCode != 0 ) // Indicates a command line error
      return returnCode;

  // if set on the command line then 'height' is now set at this point
  if( !height.empty() )
      std::cout << "height: " << height << std::endl;

  return session.run(argc, argv);
}
