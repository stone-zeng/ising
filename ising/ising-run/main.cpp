#include <iostream>
#include <stdexcept>
#include <string>

#include <include/argagg/argagg.hpp>

#include "ising-core/ising.h"
#include "ising-core/ising-parameter.h"

using namespace std;

ISING_NAMESPACE_BEGIN

const argagg::parser kIsingArgParser
{ {
    {
        "exact",
        { "--exact", "-e" },
        "Calculate specific heat for finite size 2D Ising model",
        0
    },
    {
        "simulation",
        { "--simulation", "-m" }, // `m` for Monte Carlo.
        "Analyze critical behaviors with Monte Carlo algorithm.",
        0
    },
    {
        "lattice",
        { "--lattice", "-l" },
        "Generate lattice data with Monte Carlo algorithm.",
        0
    },
    {
        "settings",
        { "--settings", "-s" },
        "Use settings file (JSON format).",
        1
    },
    {
        "dumped",
        { "--dumped", "-d" },
        "Generate dumped data (binary) rather than text.",
        0
    },
    {
        "help",
        { "--help", "-h" },
        "Print help and exit.",
        0
    },
} };

int PrintHelp(char * exe_name)
{
    ostringstream usage;
    usage << endl
          << "Usage: " << exe_name << " [OPTIONS...]" << endl
          << endl;
    argagg::fmt_ostream fmt(cerr);
    fmt << usage.str() << kIsingArgParser;

    return EXIT_SUCCESS;
}

// TODO
int RunExact() {}
int RunSimulation() {}
int RunLattice() {}

int RunIsing(int argc, char * argv[])
{
    int exit_code = EXIT_SUCCESS;
    argagg::parser_results args = kIsingArgParser.parse(argc, argv);

    if (args["help"])
    {
        exit_code = PrintHelp(argv[0]);
        return exit_code;
    }

    if (args["settings"])
    {
        // TODO
        ising::Parameter parameter(args["settings"].as<string>(""));
        parameter.Parse();
    }

    if (args["exact"])
    {
        exit_code = RunExact();
        return exit_code;
    }

    if (args["simulation"])
    {
        exit_code = RunSimulation();
        return exit_code;
    }

    if (args["lattice"])
    {
        exit_code = RunLattice();
        return exit_code;
    }

    return exit_code;
}

ISING_NAMESPACE_END

int main(int argc, char * argv[])
{
    int exit_code = ising::RunIsing(argc, argv);
    return exit_code;
}
