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

if(! -e $svkPlotGridViewTest ) {
    print "Specify the test program.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if(! -e $spectra ) {
    print "Specify the spectra.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if(defined($overlay) && ! -e $overlay ) {
    print "Overlay does not exist!\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if( defined($output_path) && ! -d $output_path ) {
    print "Output path does not exist. Execution halted.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

if( ! -d $reference_path ) {
    print "Reference path does not exist. Execution halted.\n";
    pod2usage(-verbose=>2, -exitval=>2);
}

# This will hold all the test error messages
my $errors = "";
my $return = 0;

# This is where we will store the temporory images
my $tmp_path = "/tmp/sivic_test_images";

# Lets make sure the tmp directory does not already exist
# We need it to be empty so that we can compare all images
if( -d $tmp_path ) {
    $errors = $errors . "ERROR: Temporary output path $tmp_path exists! Please remove this directory and re-run.\n"; 
    $return = 1;
    endTesting();
}

mkdir($tmp_path, 0777) || print "ERROR: Could not create temporary directory $tmp_path\n$!";

my $test_execute = "$svkPlotGridViewTest -t RenderingTest -s $spectra -p $tmp_path\n";
my $result = system("$svkPlotGridViewTest","-t", "RenderingTest", "-s", "$spectra", "-p",  "$tmp_path");
if($result != 0 ) {
    $errors = $errors . "ERROR: Could not execute $test_execute\n"; 
    $return = 1;
}
opendir(DIR, $tmp_path) or die $!;

while ( my $file = readdir(DIR) ) {

    next if ($file =~ m/^\./);
    # First lets copy the image to the output path if specified
    if( defined( $output_path ) ) {
        $result = system( "cp", "$tmp_path/$file","$output_path/$file" );
        if( $result != 0 ) {
            $errors = $errors . "ERROR: Could copy output image $tmp_path/$file\n"; 
            $return = 1;
        }
    }
    if ( ! -r "$tmp_path/$file" ) {
        $errors = $errors . "ERROR: Could read output image $tmp_path/$file\n"; 
        $return = 1;
    }
    if ( ! -r "$reference_path/$file" ) {
        $errors = $errors . "ERROR: Could read output image $reference_path/$file\n"; 
        $return = 1;
    }
    
    $result = system("diff", "$tmp_path/$file", "$reference_path/$file");
    if( $result != 0 ) {
        $errors = $errors . "ERROR: diff failed between $reference_path/$file and $tmp_path/$file\n"; 
        $return = 1;
    }
}

$result = system("rm", "-rf", $tmp_path );
if( $result != 0 ) {
    $errors = $errors . "ERROR: Temporary output path $tmp_path could not be removed!\n"; 
    $return = 1;
    endTesting();
}

endTesting();

sub endTesting {
    if( $return != 0 ) {
        print "\n#########################################################\n\n";
        print $errors;
        print "\n#########################################################\n";
        exit( $return );
    } else {
        print "\n#########################################################\n\n";
        print "PASSED!\n";
        print "\n#########################################################\n";
        exit( $return );

    }
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

