#!/usr/bin/perl
use Cwd;

print getcwd();
my $result = system("$ARGV[0]", "$ARGV[1].idf");
my $return = 0;
my $errors = "";
if( $result != 0 ) {
    $errors = $errors . " ERROR: READ/WRITE FAILED!!\n";
    $return = 1;
}
$result = system("diff", "$ARGV[1].idf", "idf_out.idf");
if( $result != 0 ) {
    $errors = $errors . " ERROR: IDF's DIFFER!!\n";
    $return = 1;
}
$result = system("diff", "$ARGV[1].int2", "idf_out.int2");
if( $result != 0 ) {
    $errors = $errors . " ERROR: INT2's DIFFER!!\n";
    $return = 1;
}
if( $return != 0 ) {
    print "\n#########################################################\n\n";
    print $errors;
    print "\n#########################################################\n";
    exit( $return );
}
