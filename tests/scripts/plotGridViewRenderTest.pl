#!/usr/bin/perl
use Cwd;
use Getopt::Long;
use Pod::Usage;

Getopt::Long::config("bundling");
GetOptions(
    "t=s"           =>\$svkPlotGridViewTest,
    "s=s"           =>\$spectra,
    "o=s"           =>\$overlay,
    "p=s"           =>\$output_path,
    "r=s"           =>\$reference_path
) or pod2usage(-verbose=>2, -exitval=>2);

if(! -e $svkPlotGridViewTest )
{
    print "Specify the test program.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if(! -e $spectra )
{
    print "Specify the spectra.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if(defined($overlay) && ! -e $overlay )
{
    print "Overlay does not exist!\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if( ! -d $output_path )
{
    print "Output path does not exist. Execution halted.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if( ! -d $reference_path )
{
    print "Reference path does not exist. Execution halted.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}
my $result = system("$svkPlotGridViewTest", "-s $spectra", "-p $output_path");
#print getcwd();
#my $result = system("$ARGV[0]", "$ARGV[1].idf");
#my $return = 0;
#my $errors = "";
#if( $result != 0 ) {
#    $errors = $errors . " ERROR: READ/WRITE FAILED!!\n";
#    $return = 1;
#}
#$result = system("diff", "$ARGV[1].idf", "idf_out.idf");
#if( $result != 0 ) {
#    $errors = $errors . " ERROR: IDF's DIFFER!!\n";
#    $return = 1;
#}
#$result = system("diff", "$ARGV[1].int2", "idf_out.int2");
#if( $result != 0 ) {
#    $errors = $errors . " ERROR: INT2's DIFFER!!\n";
#    $return = 1;
#}

if( $return != 0 ) {
    print "\n#########################################################\n\n";
    print $errors;
    print "\n#########################################################\n";
    exit( $return );
}

###############################################################################
#
#   POD usage docs
#
###############################################################################

=head1 NAME

plotGridViewRenderTest

=head1 SYNOPSIS
    
    plotGridViewRendergTest -s spectra [-o overlay] -p output_path -r reference_path

        -t  test_executable The testing executable to run.
        -s  spectra         The spectra you want to run the test on.
        -o  overlay         The overlay you want to run the test with.
        -p  output_path     Where to output the image results.
        -r  reference_path  The path for reference images to compare to.

=head1 DESCRIPTION 

    This script tests svkPlotGridView's rendering of a given dataset.

=cut

