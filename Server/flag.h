#ifndef FLAG_H
#define FLAG_H

enum class Flag
{
    Empty,  // no flag
    Init,   // init table on client
    Save,   // save file on server
    Load    // load files on client
};

#endif // FLAG_H
