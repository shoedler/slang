try import a
catch print error // [expect] Whoops!

try import b from "some/path/thats/not/real"
catch print error // [expect] Could not import module 'b'. File 'some/path/thats/not/real' does not exist.

print "still running" // [expect] still running