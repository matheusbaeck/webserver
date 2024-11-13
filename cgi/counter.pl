#!/usr/local/bin/perl
print "Content-type: text/plain", "\n\n";
$count_file = "/home/glacroix/Documents/pushed-webserv/cgi/count.txt";
if (open (FILE, "<" . $count_file)) {
        $no_accesses = <FILE>;
        close (FILE);
        if (open (FILE, ">" . $count_file)) {
            $no_accesses++;
            print FILE $no_accesses;
            close (FILE);
            print $no_accesses;
        } else {
            print "[ Can't write to the data file! Counter not incremented! ]", "\n";
        }
} else {
        print "[ Sorry! Can't read from the counter data file ]", "\n";
}
exit (0);
