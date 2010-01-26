/*
 *  Copyright © 2009-2010 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name
 *      "The Regents of the University of California," or the names of any of its
 *      contributors may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 *  OF SUCH DAMAGE.
 */

/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#undef verify

#import "DicomSeries.h"
#import "BrowserController.h"
#import "SIVIC_MRSIFilter.h"
#import <sivicApp.h>


@implementation SIVIC_MRSIFilter

- (void) initPlugin
{
}
  
- (long) filterImage:(NSString*) menuName
{
	
	/*
     * Capture the user's selection from the BrowserController:
     */
    NSArray *currentSelection = [[BrowserController currentBrowser] databaseSelection];

    printf("Number of browser selections: %d\n", [currentSelection count]);
	
    /*
     *  Iterate over the currentSelection NSArray. For each object in NSArray,
     *  get the corresponding "paths".  Paths are an NSSet.  For each NSSet,
     *  iterate over the objects it's comprised of to get the path description.
     */
    NSEnumerator *enumerator = [currentSelection objectEnumerator];
    id obj;
	NSMutableArray *myFNArray = [[NSMutableArray alloc] initWithCapacity:3];
	while ( obj = [enumerator nextObject] ) {
		
        NSEnumerator *setEnum;
        NSNumber   *element;
        setEnum = [ [(DicomSeries*)obj paths] objectEnumerator];
		int index = 0; 
        //while ((element = [setEnum nextObject]) != nil) {
		// Just get the first image in the series object: 
		if ((element = [setEnum nextObject]) != nil) {
            printf ("  Path to DICOM images: %s \n", [[element description] UTF8String]);
			NSString* path = [element description];
			NSString* argPath = [@"" stringByAppendingFormat:@"%@", path];
 
			printf ("image path arg: %s \n", [argPath UTF8String]); 
			[myFNArray insertObject: argPath atIndex:index];
			index++; 
        } 
    }  
	
	//Determine if the plugin is installed at the system or user level
 	NSString* sivicPath = @"/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources/sivic.sh";
	NSLog(@"TRY TO OPEN");
	NSLog(sivicPath);

	ifstream file( [sivicPath UTF8String] );
	if (!file) {
		register struct passwd *psswd;
		register uid_t uid;
		uid = geteuid ();
		psswd = getpwuid (uid);
		string userName;
		if (psswd) {
			userName.assign(psswd->pw_name);
		}
		cout << "USER: " << userName << endl;
		NSString *nsUserNameString = [[NSString alloc] initWithCString:userName.c_str()];		 
		
		sivicPath = [@"" stringByAppendingFormat:@"/Users/%@/Library/Application Support/OsiriX/Plugins/SIVIC_MRSI.osirixplugin/Contents/Resources/sivic.sh", nsUserNameString];
	}
	cout << "SIVIC: " << [sivicPath UTF8String] << endl;	
	NSLog(@"PATH TO USE");
	NSLog(sivicPath);
	 
	if ( [myFNArray count] == 0 ) {
		[NSTask launchedTaskWithLaunchPath: sivicPath arguments:[NSArray arrayWithObjects:nil]];
	} else if ( [myFNArray count] == 1 ) {
		[NSTask launchedTaskWithLaunchPath: sivicPath arguments:[NSArray arrayWithObjects:[myFNArray objectAtIndex:0], nil]];
	} else if ( [myFNArray count] == 2 ) {
		[NSTask launchedTaskWithLaunchPath: sivicPath arguments:[NSArray arrayWithObjects:[myFNArray objectAtIndex:0], [myFNArray objectAtIndex:1], nil]];
	} 
			
	return 0; // No Errors

}

@end
