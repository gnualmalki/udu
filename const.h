#ifndef VERSION
    #define VERSION "unknown"
#endif

static const char *USAGE =
  "Usage: udu [option(s)...] [path(s)...]\n"
  " Extremely fast disk-usage analyzer with a parallel traversal engine.\n\n"

  " OPTIONS:\n"
  "  -a, --apparent-size    show file sizes instead of disk usage\n"
  "                          (apparent = bytes reported by the filesystem,\n"
  "                           disk usage = actual space allocated)\n"
  "  -h, --help             display this help and exit\n"
  "  -q, --quiet            display output only at program exit (default)\n"
  "  -v, --verbose          display each processed file\n"
  "  -t, --tree             mimic the output of 'tree' command\n"
  "      --version          display version information and exit\n"
  "  -X, --exclude=PATTERN  skip files or directories that match a glob pattern\n"
  "                          *        any characters\n"
  "                          ?        a single character\n"
  "                          [abc]    any character in the set\n"
  "                          Examples: '*.log', 'temp?', '[0-9]*'\n"
  " EXAMPLE:\n"
  "  udu ~/ -avX epstein-files\n\n"
  "Report bugs at <https://github.com/makestatic/udu/issues>\n";

static const char *LICENSE =
  "Copyright (C) 2023, 2024, 2025  Ali Almalki <gnualmalki@gmail.com>\n"
  "License GPLv3+: GNU GPL version 3 or later "
  "<https://gnu.org/licenses/gpl.html>\n"
  "This is free software: you may change and redistribute it.\n"
  "There is NO WARRANTY, to the extent permitted by law.\n";
