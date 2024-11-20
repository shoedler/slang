try import a
catch print error // [Expect] Whoops!

try import b from "some/path/thats/not/real"
catch print error // [Expect] Could not import module 'b'. File 'some/path/thats/not/real' does not exist.

print "still running" // [Expect] still running