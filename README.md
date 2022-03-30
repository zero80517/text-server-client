# text-server-client

## Simple server client on Qt to transfer text files

The server saves the client's text files to ``SavedFiles`` and inserts the creation's date/time, filename and a link to this file in ``Table.txt``.
The client can load saved text files by selecting a name in the filename column.
The selected names will be loaded to ``LoadTable.txt`` file, where the file names will be written.
