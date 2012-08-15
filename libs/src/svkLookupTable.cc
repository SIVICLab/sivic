/*
 *  Copyright © 2009-2012 The Regents of the University of California.
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


#include <svkLookupTable.h>
#include <vtkImageAccumulate.h>


using namespace svk;


vtkCxxRevisionMacro(svkLookupTable, "$Rev$");
vtkStandardNewMacro(svkLookupTable);


const int svkLookupTable::NUM_COLORS = 1024;


//  Constructor:  gets range from ScalarImageData object.
svkLookupTable::svkLookupTable()
{
#if VTK_DEBUG_ON
    //this->DebugOn();
#endif

    this->alphaThresholdPercentage = 0.00; 
    this->reverseThreshold = false;
}


//  Constructor:  gets range from ScalarImageData object.
svkLookupTable::~svkLookupTable()
{
}


//
void svkLookupTable::SetLUTType(svkLookupTableType type)
{
	this->reverseThreshold = false;
    if ( type == svkLookupTable::COLOR ) {

        this->SetNumberOfColors(svkLookupTable::NUM_COLORS);
        this->SetNumberOfTableValues(svkLookupTable::NUM_COLORS);
        this->SetValueRange(0,1);
        this->SetHueRange(0,1);
        this->SetSaturationRange(1,1);
        this->SetAlphaRange(1.,1.);

    } else if ( type == svkLookupTable::REVERSE_COLOR ) {
        this->SetNumberOfColors(svkLookupTable::NUM_COLORS);
        this->SetNumberOfTableValues(svkLookupTable::NUM_COLORS);
        this->SetValueRange(1,0);
        this->SetHueRange(1,0);
        this->SetSaturationRange(1,1);
        this->SetAlphaRange(1.,1.);
        this->reverseThreshold = true;

    } else if ( type == svkLookupTable::GREY_SCALE ) {
        this->SetNumberOfColors(svkLookupTable::NUM_COLORS);
        this->SetNumberOfTableValues(svkLookupTable::NUM_COLORS);
        this->SetValueRange(0,1);
        this->SetHueRange(0,0);
        this->SetSaturationRange(0,0);
        this->SetAlphaRange(1.,1.);
    } else if ( type == svkLookupTable::HURD ) {

        this->SetNumberOfColors(256);
        this->SetNumberOfTableValues(256);

//cout << "HURD!" << endl;
        SetTableValue(0, 0./255, 0./255, 0./255);
        SetTableValue(1, 0./255, 0./255, 1./255);
        SetTableValue(2, 0./255, 0./255, 3./255);
        SetTableValue(3, 0./255, 0./255, 4./255);
        SetTableValue(4, 0./255, 0./255, 6./255);
        SetTableValue(5, 0./255, 0./255, 7./255);
        SetTableValue(6, 0./255, 0./255, 9./255);
        SetTableValue(7, 0./255, 0./255, 11./255);
        SetTableValue(8, 0./255, 0./255, 12./255);
        SetTableValue(9, 0./255, 0./255, 14./255);
        SetTableValue(10, 0./255, 0./255, 15./255);
        SetTableValue(11, 0./255, 0./255, 17./255);
        SetTableValue(12, 0./255, 0./255, 19./255);
        SetTableValue(13, 0./255, 0./255, 20./255);
        SetTableValue(14, 0./255, 0./255, 22./255);
        SetTableValue(15, 0./255, 0./255, 23./255);
        SetTableValue(16, 0./255, 0./255, 25./255);
        SetTableValue(17, 0./255, 0./255, 27./255);
        SetTableValue(18, 0./255, 0./255, 28./255);
        SetTableValue(19, 0./255, 0./255, 30./255);
        SetTableValue(20, 0./255, 0./255, 31./255);
        SetTableValue(21, 0./255, 0./255, 33./255);
        SetTableValue(22, 0./255, 0./255, 35./255);
        SetTableValue(23, 0./255, 0./255, 36./255);
        SetTableValue(24, 0./255, 0./255, 38./255);
        SetTableValue(25, 0./255, 0./255, 39./255);
        SetTableValue(26, 0./255, 0./255, 41./255);
        SetTableValue(27, 0./255, 0./255, 43./255);
        SetTableValue(28, 0./255, 0./255, 44./255);
        SetTableValue(29, 0./255, 0./255, 46./255);
        SetTableValue(30, 0./255, 0./255, 47./255);
        SetTableValue(31, 0./255, 0./255, 49./255);
        SetTableValue(32, 0./255, 0./255, 51./255);
        SetTableValue(33, 0./255, 0./255, 52./255);
        SetTableValue(34, 0./255, 0./255, 54./255);
        SetTableValue(35, 0./255, 0./255, 55./255);
        SetTableValue(36, 0./255, 0./255, 57./255);
        SetTableValue(37, 0./255, 0./255, 58./255);
        SetTableValue(38, 0./255, 0./255, 60./255);
        SetTableValue(39, 0./255, 0./255, 62./255);
        SetTableValue(40, 0./255, 0./255, 63./255);
        SetTableValue(41, 0./255, 0./255, 65./255);
        SetTableValue(42, 0./255, 0./255, 66./255);
        SetTableValue(43, 0./255, 0./255, 68./255);
        SetTableValue(44, 0./255, 0./255, 70./255);
        SetTableValue(45, 0./255, 0./255, 71./255);
        SetTableValue(46, 0./255, 0./255, 73./255);
        SetTableValue(47, 0./255, 0./255, 74./255);
        SetTableValue(48, 0./255, 0./255, 76./255);
        SetTableValue(49, 0./255, 0./255, 78./255);
        SetTableValue(50, 0./255, 0./255, 79./255);
        SetTableValue(51, 0./255, 0./255, 81./255);
        SetTableValue(52, 0./255, 0./255, 82./255);
        SetTableValue(53, 0./255, 0./255, 84./255);
        SetTableValue(54, 0./255, 0./255, 86./255);
        SetTableValue(55, 0./255, 0./255, 87./255);
        SetTableValue(56, 0./255, 0./255, 89./255);
        SetTableValue(57, 0./255, 0./255, 90./255);
        SetTableValue(58, 0./255, 0./255, 92./255);
        SetTableValue(59, 0./255, 0./255, 94./255);
        SetTableValue(60, 0./255, 0./255, 95./255);
        SetTableValue(61, 0./255, 0./255, 97./255);
        SetTableValue(62, 0./255, 0./255, 98./255);
        SetTableValue(63, 0./255, 0./255, 102./255);
        SetTableValue(64, 0./255, 0./255, 102./255);
        SetTableValue(65, 0./255, 3./255, 100./255);
        SetTableValue(66, 0./255, 6./255, 98./255);
        SetTableValue(67, 0./255, 9./255, 97./255);
        SetTableValue(68, 0./255, 12./255, 95./255);
        SetTableValue(69, 0./255, 15./255, 94./255);
        SetTableValue(70, 0./255, 19./255, 92./255);
        SetTableValue(71, 0./255, 22./255, 90./255);
        SetTableValue(72, 0./255, 25./255, 89./255);
        SetTableValue(73, 0./255, 28./255, 87./255);
        SetTableValue(74, 0./255, 31./255, 86./255);
        SetTableValue(75, 0./255, 35./255, 84./255);
        SetTableValue(76, 0./255, 38./255, 82./255);
        SetTableValue(77, 0./255, 41./255, 81./255);
        SetTableValue(78, 0./255, 44./255, 79./255);
        SetTableValue(79, 0./255, 47./255, 78./255);
        SetTableValue(80, 0./255, 51./255, 76./255);
        SetTableValue(81, 0./255, 54./255, 74./255);
        SetTableValue(82, 0./255, 57./255, 73./255);
        SetTableValue(83, 0./255, 60./255, 71./255);
        SetTableValue(84, 0./255, 63./255, 70./255);
        SetTableValue(85, 0./255, 66./255, 68./255);
        SetTableValue(86, 0./255, 70./255, 66./255);
        SetTableValue(87, 0./255, 73./255, 65./255);
        SetTableValue(88, 0./255, 76./255, 63./255);
        SetTableValue(89, 0./255, 79./255, 62./255);
        SetTableValue(90, 0./255, 82./255, 60./255);
        SetTableValue(91, 0./255, 86./255, 58./255);
        SetTableValue(92, 0./255, 89./255, 57./255);
        SetTableValue(93, 0./255, 92./255, 55./255);
        SetTableValue(94, 0./255, 95./255, 54./255);
        SetTableValue(95, 0./255, 98./255, 52./255);
        SetTableValue(96, 0./255, 102./255, 51./255);
        SetTableValue(97, 0./255, 105./255, 49./255);
        SetTableValue(98, 0./255, 108./255, 47./255);
        SetTableValue(99, 0./255, 111./255, 46./255);
        SetTableValue(100, 0./255, 114./255, 44./255);
        SetTableValue(101, 0./255, 117./255, 43./255);
        SetTableValue(102, 0./255, 121./255, 41./255);
        SetTableValue(103, 0./255, 124./255, 39./255);
        SetTableValue(104, 0./255, 127./255, 38./255);
        SetTableValue(105, 0./255, 130./255, 36./255);
        SetTableValue(106, 0./255, 133./255, 35./255);
        SetTableValue(107, 0./255, 137./255, 33./255);
        SetTableValue(108, 0./255, 140./255, 31./255);
        SetTableValue(109, 0./255, 143./255, 30./255);
        SetTableValue(110, 0./255, 146./255, 28./255);
        SetTableValue(111, 0./255, 149./255, 27./255);
        SetTableValue(112, 0./255, 153./255, 25./255);
        SetTableValue(113, 0./255, 156./255, 23./255);
        SetTableValue(114, 0./255, 159./255, 22./255);
        SetTableValue(115, 0./255, 162./255, 20./255);
        SetTableValue(116, 0./255, 165./255, 19./255);
        SetTableValue(117, 0./255, 168./255, 17./255);
        SetTableValue(118, 0./255, 172./255, 15./255);
        SetTableValue(119, 0./255, 175./255, 14./255);
        SetTableValue(120, 0./255, 178./255, 12./255);
        SetTableValue(121, 0./255, 181./255, 11./255);
        SetTableValue(122, 0./255, 184./255, 9./255);
        SetTableValue(123, 0./255, 188./255, 7./255);
        SetTableValue(124, 0./255, 191./255, 6./255);
        SetTableValue(125, 0./255, 194./255, 4./255);
        SetTableValue(126, 0./255, 197./255, 3./255);
        SetTableValue(127, 0./255, 204./255, 0./255);
        SetTableValue(128, 0./255, 204./255, 0./255);
        SetTableValue(129, 3./255, 204./255, 0./255);
        SetTableValue(130, 7./255, 205./255, 0./255);
        SetTableValue(131, 11./255, 206./255, 0./255);
        SetTableValue(132, 15./255, 207./255, 0./255);
        SetTableValue(133, 19./255, 207./255, 0./255);
        SetTableValue(134, 23./255, 208./255, 0./255);
        SetTableValue(135, 27./255, 209./255, 0./255);
        SetTableValue(136, 31./255, 210./255, 0./255);
        SetTableValue(137, 35./255, 211./255, 0./255);
        SetTableValue(138, 39./255, 211./255, 0./255);
        SetTableValue(139, 43./255, 212./255, 0./255);
        SetTableValue(140, 47./255, 213./255, 0./255);
        SetTableValue(141, 51./255, 214./255, 0./255);
        SetTableValue(142, 55./255, 215./255, 0./255);
        SetTableValue(143, 59./255, 215./255, 0./255);
        SetTableValue(144, 63./255, 216./255, 0./255);
        SetTableValue(145, 67./255, 217./255, 0./255);
        SetTableValue(146, 71./255, 218./255, 0./255);
        SetTableValue(147, 75./255, 219./255, 0./255);
        SetTableValue(148, 79./255, 219./255, 0./255);
        SetTableValue(149, 83./255, 220./255, 0./255);
        SetTableValue(150, 87./255, 221./255, 0./255);
        SetTableValue(151, 91./255, 222./255, 0./255);
        SetTableValue(152, 95./255, 223./255, 0./255);
        SetTableValue(153, 99./255, 223./255, 0./255);
        SetTableValue(154, 103./255, 224./255, 0./255);
        SetTableValue(155, 107./255, 225./255, 0./255);
        SetTableValue(156, 111./255, 226./255, 0./255);
        SetTableValue(157, 115./255, 227./255, 0./255);
        SetTableValue(158, 119./255, 227./255, 0./255);
        SetTableValue(159, 123./255, 228./255, 0./255);
        SetTableValue(160, 127./255, 229./255, 0./255);
        SetTableValue(161, 131./255, 230./255, 0./255);
        SetTableValue(162, 135./255, 231./255, 0./255);
        SetTableValue(163, 139./255, 231./255, 0./255);
        SetTableValue(164, 143./255, 232./255, 0./255);
        SetTableValue(165, 147./255, 233./255, 0./255);
        SetTableValue(166, 151./255, 234./255, 0./255);
        SetTableValue(167, 155./255, 235./255, 0./255);
        SetTableValue(168, 159./255, 235./255, 0./255);
        SetTableValue(169, 163./255, 236./255, 0./255);
        SetTableValue(170, 167./255, 237./255, 0./255);
        SetTableValue(171, 171./255, 238./255, 0./255);
        SetTableValue(172, 175./255, 239./255, 0./255);
        SetTableValue(173, 179./255, 239./255, 0./255);
        SetTableValue(174, 183./255, 240./255, 0./255);
        SetTableValue(175, 187./255, 241./255, 0./255);
        SetTableValue(176, 191./255, 242./255, 0./255);
        SetTableValue(177, 195./255, 243./255, 0./255);
        SetTableValue(178, 199./255, 243./255, 0./255);
        SetTableValue(179, 203./255, 244./255, 0./255);
        SetTableValue(180, 207./255, 245./255, 0./255);
        SetTableValue(181, 211./255, 246./255, 0./255);
        SetTableValue(182, 215./255, 247./255, 0./255);
        SetTableValue(183, 219./255, 247./255, 0./255);
        SetTableValue(184, 223./255, 248./255, 0./255);
        SetTableValue(185, 227./255, 249./255, 0./255);
        SetTableValue(186, 231./255, 250./255, 0./255);
        SetTableValue(187, 235./255, 251./255, 0./255);
        SetTableValue(188, 239./255, 251./255, 0./255);
        SetTableValue(189, 243./255, 252./255, 0./255);
        SetTableValue(190, 247./255, 253./255, 0./255);
        SetTableValue(191, 255./255, 255./255, 0./255);
        SetTableValue(192, 255./255, 255./255, 0./255);
        SetTableValue(193, 255./255, 251./255, 0./255);
        SetTableValue(194, 255./255, 247./255, 0./255);
        SetTableValue(195, 255./255, 243./255, 0./255);
        SetTableValue(196, 255./255, 239./255, 0./255);
        SetTableValue(197, 255./255, 235./255, 0./255);
        SetTableValue(198, 255./255, 231./255, 0./255);
        SetTableValue(199, 255./255, 227./255, 0./255);
        SetTableValue(200, 255./255, 223./255, 0./255);
        SetTableValue(201, 255./255, 219./255, 0./255);
        SetTableValue(202, 255./255, 215./255, 0./255);
        SetTableValue(203, 255./255, 211./255, 0./255);
        SetTableValue(204, 255./255, 207./255, 0./255);
        SetTableValue(205, 255./255, 203./255, 0./255);
        SetTableValue(206, 255./255, 199./255, 0./255);
        SetTableValue(207, 255./255, 195./255, 0./255);
        SetTableValue(208, 255./255, 191./255, 0./255);
        SetTableValue(209, 255./255, 187./255, 0./255);
        SetTableValue(210, 255./255, 183./255, 0./255);
        SetTableValue(211, 255./255, 179./255, 0./255);
        SetTableValue(212, 255./255, 175./255, 0./255);
        SetTableValue(213, 255./255, 171./255, 0./255);
        SetTableValue(214, 255./255, 167./255, 0./255);
        SetTableValue(215, 255./255, 163./255, 0./255);
        SetTableValue(216, 255./255, 159./255, 0./255);
        SetTableValue(217, 255./255, 155./255, 0./255);
        SetTableValue(218, 255./255, 151./255, 0./255);
        SetTableValue(219, 255./255, 147./255, 0./255);
        SetTableValue(220, 255./255, 143./255, 0./255);
        SetTableValue(221, 255./255, 139./255, 0./255);
        SetTableValue(222, 255./255, 135./255, 0./255);
        SetTableValue(223, 255./255, 131./255, 0./255);
        SetTableValue(224, 255./255, 127./255, 0./255);
        SetTableValue(225, 255./255, 123./255, 0./255);
        SetTableValue(226, 255./255, 119./255, 0./255);
        SetTableValue(227, 255./255, 115./255, 0./255);
        SetTableValue(228, 255./255, 111./255, 0./255);
        SetTableValue(229, 255./255, 107./255, 0./255);
        SetTableValue(230, 255./255, 103./255, 0./255);
        SetTableValue(231, 255./255, 99./255, 0./255);
        SetTableValue(232, 255./255, 95./255, 0./255);
        SetTableValue(233, 255./255, 91./255, 0./255);
        SetTableValue(234, 255./255, 87./255, 0./255);
        SetTableValue(235, 255./255, 83./255, 0./255);
        SetTableValue(236, 255./255, 79./255, 0./255);
        SetTableValue(237, 255./255, 75./255, 0./255);
        SetTableValue(238, 255./255, 71./255, 0./255);
        SetTableValue(239, 255./255, 67./255, 0./255);
        SetTableValue(240, 255./255, 63./255, 0./255);
        SetTableValue(241, 255./255, 59./255, 0./255);
        SetTableValue(242, 255./255, 55./255, 0./255);
        SetTableValue(243, 255./255, 51./255, 0./255);
        SetTableValue(244, 255./255, 47./255, 0./255);
        SetTableValue(245, 255./255, 43./255, 0./255);
        SetTableValue(246, 255./255, 39./255, 0./255);
        SetTableValue(247, 255./255, 35./255, 0./255);
        SetTableValue(248, 255./255, 31./255, 0./255);
        SetTableValue(249, 255./255, 27./255, 0./255);
        SetTableValue(250, 255./255, 23./255, 0./255);
        SetTableValue(251, 255./255, 19./255, 0./255);
        SetTableValue(252, 255./255, 15./255, 0./255);
        SetTableValue(253, 255./255, 11./255, 0./255);
        SetTableValue(254, 255./255, 7./255, 0./255);
        SetTableValue(255, 255./255, 0./255, 0./255);

    } else if ( type == svkLookupTable::CYAN_HOT ) {

//cout << "CYAN!" << endl;
        this->SetNumberOfColors(256);
        this->SetNumberOfTableValues(256);
        SetTableValue(0,  0./255,  0./255,  0./255);
        SetTableValue(1,  0./255,  1./255,  2./255);
        SetTableValue(2,  0./255,  2./255,  5./255);
        SetTableValue(3,  0./255,  4./255,  8./255);
        SetTableValue(4,  0./255,  5./255,  11./255);
        SetTableValue(5,  0./255,  7./255,  14./255);
        SetTableValue(6,  0./255,  8./255,  17./255);
        SetTableValue(7,  0./255,  10./255,  20./255);
        SetTableValue(8,  0./255,  11./255,  23./255);
        SetTableValue(9,  0./255,  12./255,  26./255);
        SetTableValue(10,  0./255,  14./255,  29./255);
        SetTableValue(11,  0./255,  15./255,  32./255);
        SetTableValue(12,  0./255,  17./255,  35./255);
        SetTableValue(13,  0./255,  18./255,  38./255);
        SetTableValue(14,  0./255,  20./255,  41./255);
        SetTableValue(15,  0./255,  21./255,  44./255);
        SetTableValue(16,  0./255,  23./255,  47./255);
        SetTableValue(17,  0./255,  24./255,  50./255);
        SetTableValue(18,  0./255,  25./255,  53./255);
        SetTableValue(19,  0./255,  27./255,  56./255);
        SetTableValue(20,  0./255,  28./255,  59./255);
        SetTableValue(21,  0./255,  30./255,  62./255);
        SetTableValue(22,  0./255,  31./255,  65./255);
        SetTableValue(23,  0./255,  33./255,  68./255);
        SetTableValue(24,  0./255,  34./255,  71./255);
        SetTableValue(25,  0./255,  36./255,  74./255);
        SetTableValue(26,  0./255,  37./255,  77./255);
        SetTableValue(27,  0./255,  38./255,  80./255);
        SetTableValue(28,  0./255,  40./255,  83./255);
        SetTableValue(29,  0./255,  41./255,  86./255);
        SetTableValue(30,  0./255,  43./255,  89./255);
        SetTableValue(31,  0./255,  44./255,  92./255);
        SetTableValue(32,  0./255,  46./255,  95./255);
        SetTableValue(33,  0./255,  47./255,  98./255);
        SetTableValue(34,  0./255,  49./255,  101./255);
        SetTableValue(35,  0./255,  50./255,  104./255);
        SetTableValue(36,  0./255,  51./255,  107./255);
        SetTableValue(37,  0./255,  53./255,  110./255);
        SetTableValue(38,  0./255,  54./255,  113./255);
        SetTableValue(39,  0./255,  56./255,  116./255);
        SetTableValue(40,  0./255,  57./255,  119./255);
        SetTableValue(41,  0./255,  59./255,  122./255);
        SetTableValue(42,  0./255,  60./255,  125./255);
        SetTableValue(43,  0./255,  61./255,  128./255);
        SetTableValue(44,  0./255,  63./255,  131./255);
        SetTableValue(45,  0./255,  64./255,  134./255);
        SetTableValue(46,  0./255,  66./255,  137./255);
        SetTableValue(47,  0./255,  67./255,  140./255);
        SetTableValue(48,  0./255,  69./255,  143./255);
        SetTableValue(49,  0./255,  70./255,  146./255);
        SetTableValue(50,  0./255,  72./255,  149./255);
        SetTableValue(51,  0./255,  73./255,  152./255);
        SetTableValue(52,  0./255,  74./255,  155./255);
        SetTableValue(53,  0./255,  76./255,  158./255);
        SetTableValue(54,  0./255,  77./255,  161./255);
        SetTableValue(55,  0./255,  79./255,  164./255);
        SetTableValue(56,  0./255,  80./255,  167./255);
        SetTableValue(57,  0./255,  82./255,  170./255);
        SetTableValue(58,  0./255,  83./255,  173./255);
        SetTableValue(59,  0./255,  85./255,  176./255);
        SetTableValue(60,  0./255,  86./255,  179./255);
        SetTableValue(61,  0./255,  87./255,  182./255);
        SetTableValue(62,  0./255,  89./255,  185./255);
        SetTableValue(63,  0./255,  90./255,  188./255);
        SetTableValue(64,  0./255,  92./255,  191./255);
        SetTableValue(65,  0./255,  93./255,  194./255);
        SetTableValue(66,  0./255,  95./255,  197./255);
        SetTableValue(67,  0./255,  96./255,  200./255);
        SetTableValue(68,  0./255,  98./255,  203./255);
        SetTableValue(69,  0./255,  99./255,  206./255);
        SetTableValue(70,  0./255,  100./255,  209./255);
        SetTableValue(71,  0./255,  102./255,  212./255);
        SetTableValue(72,  0./255,  103./255,  215./255);
        SetTableValue(73,  0./255,  105./255,  218./255);
        SetTableValue(74,  0./255,  106./255,  221./255);
        SetTableValue(75,  0./255,  108./255,  224./255);
        SetTableValue(76,  0./255,  109./255,  227./255);
        SetTableValue(77,  0./255,  110./255,  230./255);
        SetTableValue(78,  0./255,  112./255,  233./255);
        SetTableValue(79,  0./255,  113./255,  236./255);
        SetTableValue(80,  0./255,  115./255,  239./255);
        SetTableValue(81,  0./255,  116./255,  242./255);
        SetTableValue(82,  0./255,  118./255,  245./255);
        SetTableValue(83,  0./255,  119./255,  248./255);
        SetTableValue(84,  0./255,  123./255,  255./255);
        SetTableValue(85,  0./255,  123./255,  255./255);
        SetTableValue(86,  0./255,  124./255,  255./255);
        SetTableValue(87,  0./255,  126./255,  255./255);
        SetTableValue(88,  0./255,  127./255,  255./255);
        SetTableValue(89,  0./255,  129./255,  255./255);
        SetTableValue(90,  0./255,  130./255,  255./255);
        SetTableValue(91,  0./255,  132./255,  255./255);
        SetTableValue(92,  0./255,  133./255,  255./255);
        SetTableValue(93,  0./255,  135./255,  255./255);
        SetTableValue(94,  0./255,  136./255,  255./255);
        SetTableValue(95,  0./255,  138./255,  255./255);
        SetTableValue(96,  0./255,  140./255,  255./255);
        SetTableValue(97,  0./255,  141./255,  255./255);
        SetTableValue(98,  0./255,  143./255,  255./255);
        SetTableValue(99,  0./255,  144./255,  255./255);
        SetTableValue(100,  0./255,  146./255,  255./255);
        SetTableValue(101,  0./255,  147./255,  255./255);
        SetTableValue(102,  0./255,  149./255,  255./255);
        SetTableValue(103,  0./255,  150./255,  255./255);
        SetTableValue(104,  0./255,  152./255,  255./255);
        SetTableValue(105,  0./255,  153./255,  255./255);
        SetTableValue(106,  0./255,  155./255,  255./255);
        SetTableValue(107,  0./255,  157./255,  255./255);
        SetTableValue(108,  0./255,  158./255,  255./255);
        SetTableValue(109,  0./255,  160./255,  255./255);
        SetTableValue(110,  0./255,  161./255,  255./255);
        SetTableValue(111,  0./255,  163./255,  255./255);
        SetTableValue(112,  0./255,  164./255,  255./255);
        SetTableValue(113,  0./255,  166./255,  255./255);
        SetTableValue(114,  0./255,  167./255,  255./255);
        SetTableValue(115,  0./255,  169./255,  255./255);
        SetTableValue(116,  0./255,  170./255,  255./255);
        SetTableValue(117,  0./255,  172./255,  255./255);
        SetTableValue(118,  0./255,  174./255,  255./255);
        SetTableValue(119,  0./255,  175./255,  255./255);
        SetTableValue(120,  0./255,  177./255,  255./255);
        SetTableValue(121,  0./255,  178./255,  255./255);
        SetTableValue(122,  0./255,  180./255,  255./255);
        SetTableValue(123,  0./255,  181./255,  255./255);
        SetTableValue(124,  0./255,  183./255,  255./255);
        SetTableValue(125,  0./255,  184./255,  255./255);
        SetTableValue(126,  0./255,  186./255,  255./255);
        SetTableValue(127,  0./255,  187./255,  255./255);
        SetTableValue(128,  0./255,  189./255,  255./255);
        SetTableValue(129,  0./255,  191./255,  255./255);
        SetTableValue(130,  0./255,  192./255,  255./255);
        SetTableValue(131,  0./255,  194./255,  255./255);
        SetTableValue(132,  0./255,  195./255,  255./255);
        SetTableValue(133,  0./255,  197./255,  255./255);
        SetTableValue(134,  0./255,  198./255,  255./255);
        SetTableValue(135,  0./255,  200./255,  255./255);
        SetTableValue(136,  0./255,  201./255,  255./255);
        SetTableValue(137,  0./255,  203./255,  255./255);
        SetTableValue(138,  0./255,  204./255,  255./255);
        SetTableValue(139,  0./255,  206./255,  255./255);
        SetTableValue(140,  0./255,  208./255,  255./255);
        SetTableValue(141,  0./255,  209./255,  255./255);
        SetTableValue(142,  0./255,  211./255,  255./255);
        SetTableValue(143,  0./255,  212./255,  255./255);
        SetTableValue(144,  0./255,  214./255,  255./255);
        SetTableValue(145,  0./255,  215./255,  255./255);
        SetTableValue(146,  0./255,  217./255,  255./255);
        SetTableValue(147,  0./255,  218./255,  255./255);
        SetTableValue(148,  0./255,  220./255,  255./255);
        SetTableValue(149,  0./255,  222./255,  255./255);
        SetTableValue(150,  0./255,  223./255,  255./255);
        SetTableValue(151,  0./255,  225./255,  255./255);
        SetTableValue(152,  0./255,  226./255,  255./255);
        SetTableValue(153,  0./255,  228./255,  255./255);
        SetTableValue(154,  0./255,  229./255,  255./255);
        SetTableValue(155,  0./255,  231./255,  255./255);
        SetTableValue(156,  0./255,  232./255,  255./255);
        SetTableValue(157,  0./255,  234./255,  255./255);
        SetTableValue(158,  0./255,  235./255,  255./255);
        SetTableValue(159,  0./255,  237./255,  255./255);
        SetTableValue(160,  0./255,  239./255,  255./255);
        SetTableValue(161,  0./255,  240./255,  255./255);
        SetTableValue(162,  0./255,  242./255,  255./255);
        SetTableValue(163,  0./255,  243./255,  255./255);
        SetTableValue(164,  0./255,  245./255,  255./255);
        SetTableValue(165,  0./255,  246./255,  255./255);
        SetTableValue(166,  0./255,  248./255,  255./255);
        SetTableValue(167,  0./255,  249./255,  255./255);
        SetTableValue(168,  0./255,  251./255,  255./255);
        SetTableValue(169,  0./255,  255./255,  255./255);
        SetTableValue(170,  0./255,  255./255,  255./255);
        SetTableValue(171,  2./255,  255./255,  255./255);
        SetTableValue(172,  5./255,  255./255,  255./255);
        SetTableValue(173,  8./255,  255./255,  255./255);
        SetTableValue(174,  11./255,  255./255,  255./255);
        SetTableValue(175,  14./255,  255./255,  255./255);
        SetTableValue(176,  17./255,  255./255,  255./255);
        SetTableValue(177,  20./255,  255./255,  255./255);
        SetTableValue(178,  23./255,  255./255,  255./255);
        SetTableValue(179,  26./255,  255./255,  255./255);
        SetTableValue(180,  29./255,  255./255,  255./255);
        SetTableValue(181,  32./255,  255./255,  255./255);
        SetTableValue(182,  35./255,  255./255,  255./255);
        SetTableValue(183,  38./255,  255./255,  255./255);
        SetTableValue(184,  41./255,  255./255,  255./255);
        SetTableValue(185,  44./255,  255./255,  255./255);
        SetTableValue(186,  47./255,  255./255,  255./255);
        SetTableValue(187,  50./255,  255./255,  255./255);
        SetTableValue(188,  53./255,  255./255,  255./255);
        SetTableValue(189,  56./255,  255./255,  255./255);
        SetTableValue(190,  59./255,  255./255,  255./255);
        SetTableValue(191,  62./255,  255./255,  255./255);
        SetTableValue(192,  65./255,  255./255,  255./255);
        SetTableValue(193,  68./255,  255./255,  255./255);
        SetTableValue(194,  71./255,  255./255,  255./255);
        SetTableValue(195,  74./255,  255./255,  255./255);
        SetTableValue(196,  77./255,  255./255,  255./255);
        SetTableValue(197,  80./255,  255./255,  255./255);
        SetTableValue(198,  83./255,  255./255,  255./255);
        SetTableValue(199,  86./255,  255./255,  255./255);
        SetTableValue(200,  89./255,  255./255,  255./255);
        SetTableValue(201,  92./255,  255./255,  255./255);
        SetTableValue(202,  95./255,  255./255,  255./255);
        SetTableValue(203,  98./255,  255./255,  255./255);
        SetTableValue(204,  101./255,  255./255,  255./255);
        SetTableValue(205,  104./255,  255./255,  255./255);
        SetTableValue(206,  107./255,  255./255,  255./255);
        SetTableValue(207,  110./255,  255./255,  255./255);
        SetTableValue(208,  113./255,  255./255,  255./255);
        SetTableValue(209,  116./255,  255./255,  255./255);
        SetTableValue(210,  119./255,  255./255,  255./255);
        SetTableValue(211,  122./255,  255./255,  255./255);
        SetTableValue(212,  125./255,  255./255,  255./255);
        SetTableValue(213,  128./255,  255./255,  255./255);
        SetTableValue(214,  131./255,  255./255,  255./255);
        SetTableValue(215,  134./255,  255./255,  255./255);
        SetTableValue(216,  137./255,  255./255,  255./255);
        SetTableValue(217,  140./255,  255./255,  255./255);
        SetTableValue(218,  143./255,  255./255,  255./255);
        SetTableValue(219,  146./255,  255./255,  255./255);
        SetTableValue(220,  149./255,  255./255,  255./255);
        SetTableValue(221,  152./255,  255./255,  255./255);
        SetTableValue(222,  155./255,  255./255,  255./255);
        SetTableValue(223,  158./255,  255./255,  255./255);
        SetTableValue(224,  161./255,  255./255,  255./255);
        SetTableValue(225,  164./255,  255./255,  255./255);
        SetTableValue(226,  167./255,  255./255,  255./255);
        SetTableValue(227,  170./255,  255./255,  255./255);
        SetTableValue(228,  173./255,  255./255,  255./255);
        SetTableValue(229,  176./255,  255./255,  255./255);
        SetTableValue(230,  179./255,  255./255,  255./255);
        SetTableValue(231,  182./255,  255./255,  255./255);
        SetTableValue(232,  185./255,  255./255,  255./255);
        SetTableValue(233,  188./255,  255./255,  255./255);
        SetTableValue(234,  191./255,  255./255,  255./255);
        SetTableValue(235,  194./255,  255./255,  255./255);
        SetTableValue(236,  197./255,  255./255,  255./255);
        SetTableValue(237,  200./255,  255./255,  255./255);
        SetTableValue(238,  203./255,  255./255,  255./255);
        SetTableValue(239,  206./255,  255./255,  255./255);
        SetTableValue(240,  209./255,  255./255,  255./255);
        SetTableValue(241,  212./255,  255./255,  255./255);
        SetTableValue(242,  215./255,  255./255,  255./255);
        SetTableValue(243,  218./255,  255./255,  255./255);
        SetTableValue(244,  221./255,  255./255,  255./255);
        SetTableValue(245,  224./255,  255./255,  255./255);
        SetTableValue(246,  227./255,  255./255,  255./255);
        SetTableValue(247,  230./255,  255./255,  255./255);
        SetTableValue(248,  233./255,  255./255,  255./255);
        SetTableValue(249,  236./255,  255./255,  255./255);
        SetTableValue(250,  239./255,  255./255,  255./255);
        SetTableValue(251,  242./255,  255./255,  255./255);
        SetTableValue(252,  245./255,  255./255,  255./255);
        SetTableValue(253,  248./255,  255./255,  255./255);
        SetTableValue(254,  251./255,  255./255,  255./255);
        SetTableValue(255,  255./255,  255./255,  255./255);
    } else if ( type == svkLookupTable::FIRE ) {

        this->SetNumberOfColors(256);
        this->SetNumberOfTableValues(256);

        SetTableValue(0, 0./255, 0./255, 0./255);
        SetTableValue(1, 0./255, 0./255, 7./255);
        SetTableValue(2, 0./255, 0./255, 15./255);
        SetTableValue(3, 0./255, 0./255, 22./255);
        SetTableValue(4, 0./255, 0./255, 30./255);
        SetTableValue(5, 0./255, 0./255, 38./255);
        SetTableValue(6, 0./255, 0./255, 45./255);
        SetTableValue(7, 0./255, 0./255, 53./255);
        SetTableValue(8, 0./255, 0./255, 61./255);
        SetTableValue(9, 0./255, 0./255, 65./255);
        SetTableValue(10, 0./255, 0./255, 69./255);
        SetTableValue(11, 0./255, 0./255, 74./255);
        SetTableValue(12, 0./255, 0./255, 78./255);
        SetTableValue(13, 0./255, 0./255, 82./255);
        SetTableValue(14, 0./255, 0./255, 87./255);
        SetTableValue(15, 0./255, 0./255, 91./255);
        SetTableValue(16, 1./255, 0./255, 96./255);
        SetTableValue(17, 4./255, 0./255, 100./255);
        SetTableValue(18, 7./255, 0./255, 104./255);
        SetTableValue(19, 10./255, 0./255, 108./255);
        SetTableValue(20, 13./255, 0./255, 113./255);
        SetTableValue(21, 16./255, 0./255, 117./255);
        SetTableValue(22, 19./255, 0./255, 121./255);
        SetTableValue(23, 22./255, 0./255, 125./255);
        SetTableValue(24, 25./255, 0./255, 130./255);
        SetTableValue(25, 28./255, 0./255, 134./255);
        SetTableValue(26, 31./255, 0./255, 138./255);
        SetTableValue(27, 34./255, 0./255, 143./255);
        SetTableValue(28, 37./255, 0./255, 147./255);
        SetTableValue(29, 40./255, 0./255, 151./255);
        SetTableValue(30, 43./255, 0./255, 156./255);
        SetTableValue(31, 46./255, 0./255, 160./255);
        SetTableValue(32, 49./255, 0./255, 165./255);
        SetTableValue(33, 52./255, 0./255, 168./255);
        SetTableValue(34, 55./255, 0./255, 171./255);
        SetTableValue(35, 58./255, 0./255, 175./255);
        SetTableValue(36, 61./255, 0./255, 178./255);
        SetTableValue(37, 64./255, 0./255, 181./255);
        SetTableValue(38, 67./255, 0./255, 185./255);
        SetTableValue(39, 70./255, 0./255, 188./255);
        SetTableValue(40, 73./255, 0./255, 192./255);
        SetTableValue(41, 76./255, 0./255, 195./255);
        SetTableValue(42, 79./255, 0./255, 199./255);
        SetTableValue(43, 82./255, 0./255, 202./255);
        SetTableValue(44, 85./255, 0./255, 206./255);
        SetTableValue(45, 88./255, 0./255, 209./255);
        SetTableValue(46, 91./255, 0./255, 213./255);
        SetTableValue(47, 94./255, 0./255, 216./255);
        SetTableValue(48, 98./255, 0./255, 220./255);
        SetTableValue(49, 101./255, 0./255, 220./255);
        SetTableValue(50, 104./255, 0./255, 221./255);
        SetTableValue(51, 107./255, 0./255, 222./255);
        SetTableValue(52, 110./255, 0./255, 223./255);
        SetTableValue(53, 113./255, 0./255, 224./255);
        SetTableValue(54, 116./255, 0./255, 225./255);
        SetTableValue(55, 119./255, 0./255, 226./255);
        SetTableValue(56, 122./255, 0./255, 227./255);
        SetTableValue(57, 125./255, 0./255, 224./255);
        SetTableValue(58, 128./255, 0./255, 222./255);
        SetTableValue(59, 131./255, 0./255, 220./255);
        SetTableValue(60, 134./255, 0./255, 218./255);
        SetTableValue(61, 137./255, 0./255, 216./255);
        SetTableValue(62, 140./255, 0./255, 214./255);
        SetTableValue(63, 143./255, 0./255, 212./255);
        SetTableValue(64, 146./255, 0./255, 210./255);
        SetTableValue(65, 148./255, 0./255, 206./255);
        SetTableValue(66, 150./255, 0./255, 202./255);
        SetTableValue(67, 152./255, 0./255, 199./255);
        SetTableValue(68, 154./255, 0./255, 195./255);
        SetTableValue(69, 156./255, 0./255, 191./255);
        SetTableValue(70, 158./255, 0./255, 188./255);
        SetTableValue(71, 160./255, 0./255, 184./255);
        SetTableValue(72, 162./255, 0./255, 181./255);
        SetTableValue(73, 163./255, 0./255, 177./255);
        SetTableValue(74, 164./255, 0./255, 173./255);
        SetTableValue(75, 166./255, 0./255, 169./255);
        SetTableValue(76, 167./255, 0./255, 166./255);
        SetTableValue(77, 168./255, 0./255, 162./255);
        SetTableValue(78, 170./255, 0./255, 158./255);
        SetTableValue(79, 171./255, 0./255, 154./255);
        SetTableValue(80, 173./255, 0./255, 151./255);
        SetTableValue(81, 174./255, 0./255, 147./255);
        SetTableValue(82, 175./255, 0./255, 143./255);
        SetTableValue(83, 177./255, 0./255, 140./255);
        SetTableValue(84, 178./255, 0./255, 136./255);
        SetTableValue(85, 179./255, 0./255, 132./255);
        SetTableValue(86, 181./255, 0./255, 129./255);
        SetTableValue(87, 182./255, 0./255, 125./255);
        SetTableValue(88, 184./255, 0./255, 122./255);
        SetTableValue(89, 185./255, 0./255, 118./255);
        SetTableValue(90, 186./255, 0./255, 114./255);
        SetTableValue(91, 188./255, 0./255, 111./255);
        SetTableValue(92, 189./255, 0./255, 107./255);
        SetTableValue(93, 190./255, 0./255, 103./255);
        SetTableValue(94, 192./255, 0./255, 100./255);
        SetTableValue(95, 193./255, 0./255, 96./255);
        SetTableValue(96, 195./255, 0./255, 93./255);
        SetTableValue(97, 196./255, 1./255, 89./255);
        SetTableValue(98, 198./255, 3./255, 85./255);
        SetTableValue(99, 199./255, 5./255, 82./255);
        SetTableValue(100, 201./255, 7./255, 78./255);
        SetTableValue(101, 202./255, 8./255, 74./255);
        SetTableValue(102, 204./255, 10./255, 71./255);
        SetTableValue(103, 205./255, 12./255, 67./255);
        SetTableValue(104, 207./255, 14./255, 64./255);
        SetTableValue(105, 208./255, 16./255, 60./255);
        SetTableValue(106, 209./255, 19./255, 56./255);
        SetTableValue(107, 210./255, 21./255, 53./255);
        SetTableValue(108, 212./255, 24./255, 49./255);
        SetTableValue(109, 213./255, 27./255, 45./255);
        SetTableValue(110, 214./255, 29./255, 42./255);
        SetTableValue(111, 215./255, 32./255, 38./255);
        SetTableValue(112, 217./255, 35./255, 35./255);
        SetTableValue(113, 218./255, 37./255, 31./255);
        SetTableValue(114, 220./255, 40./255, 27./255);
        SetTableValue(115, 221./255, 43./255, 23./255);
        SetTableValue(116, 223./255, 46./255, 20./255);
        SetTableValue(117, 224./255, 48./255, 16./255);
        SetTableValue(118, 226./255, 51./255, 12./255);
        SetTableValue(119, 227./255, 54./255, 8./255);
        SetTableValue(120, 229./255, 57./255, 5./255);
        SetTableValue(121, 230./255, 59./255, 4./255);
        SetTableValue(122, 231./255, 62./255, 3./255);
        SetTableValue(123, 233./255, 65./255, 3./255);
        SetTableValue(124, 234./255, 68./255, 2./255);
        SetTableValue(125, 235./255, 70./255, 1./255);
        SetTableValue(126, 237./255, 73./255, 1./255);
        SetTableValue(127, 238./255, 76./255, 0./255);
        SetTableValue(128, 240./255, 79./255, 0./255);
        SetTableValue(129, 241./255, 81./255, 0./255);
        SetTableValue(130, 243./255, 84./255, 0./255);
        SetTableValue(131, 244./255, 87./255, 0./255);
        SetTableValue(132, 246./255, 90./255, 0./255);
        SetTableValue(133, 247./255, 92./255, 0./255);
        SetTableValue(134, 249./255, 95./255, 0./255);
        SetTableValue(135, 250./255, 98./255, 0./255);
        SetTableValue(136, 252./255, 101./255, 0./255);
        SetTableValue(137, 252./255, 103./255, 0./255);
        SetTableValue(138, 252./255, 105./255, 0./255);
        SetTableValue(139, 253./255, 107./255, 0./255);
        SetTableValue(140, 253./255, 109./255, 0./255);
        SetTableValue(141, 253./255, 111./255, 0./255);
        SetTableValue(142, 254./255, 113./255, 0./255);
        SetTableValue(143, 254./255, 115./255, 0./255);
        SetTableValue(144, 255./255, 117./255, 0./255);
        SetTableValue(145, 255./255, 119./255, 0./255);
        SetTableValue(146, 255./255, 121./255, 0./255);
        SetTableValue(147, 255./255, 123./255, 0./255);
        SetTableValue(148, 255./255, 125./255, 0./255);
        SetTableValue(149, 255./255, 127./255, 0./255);
        SetTableValue(150, 255./255, 129./255, 0./255);
        SetTableValue(151, 255./255, 131./255, 0./255);
        SetTableValue(152, 255./255, 133./255, 0./255);
        SetTableValue(153, 255./255, 134./255, 0./255);
        SetTableValue(154, 255./255, 136./255, 0./255);
        SetTableValue(155, 255./255, 138./255, 0./255);
        SetTableValue(156, 255./255, 140./255, 0./255);
        SetTableValue(157, 255./255, 141./255, 0./255);
        SetTableValue(158, 255./255, 143./255, 0./255);
        SetTableValue(159, 255./255, 145./255, 0./255);
        SetTableValue(160, 255./255, 147./255, 0./255);
        SetTableValue(161, 255./255, 148./255, 0./255);
        SetTableValue(162, 255./255, 150./255, 0./255);
        SetTableValue(163, 255./255, 152./255, 0./255);
        SetTableValue(164, 255./255, 154./255, 0./255);
        SetTableValue(165, 255./255, 155./255, 0./255);
        SetTableValue(166, 255./255, 157./255, 0./255);
        SetTableValue(167, 255./255, 159./255, 0./255);
        SetTableValue(168, 255./255, 161./255, 0./255);
        SetTableValue(169, 255./255, 162./255, 0./255);
        SetTableValue(170, 255./255, 164./255, 0./255);
        SetTableValue(171, 255./255, 166./255, 0./255);
        SetTableValue(172, 255./255, 168./255, 0./255);
        SetTableValue(173, 255./255, 169./255, 0./255);
        SetTableValue(174, 255./255, 171./255, 0./255);
        SetTableValue(175, 255./255, 173./255, 0./255);
        SetTableValue(176, 255./255, 175./255, 0./255);
        SetTableValue(177, 255./255, 176./255, 0./255);
        SetTableValue(178, 255./255, 178./255, 0./255);
        SetTableValue(179, 255./255, 180./255, 0./255);
        SetTableValue(180, 255./255, 182./255, 0./255);
        SetTableValue(181, 255./255, 184./255, 0./255);
        SetTableValue(182, 255./255, 186./255, 0./255);
        SetTableValue(183, 255./255, 188./255, 0./255);
        SetTableValue(184, 255./255, 190./255, 0./255);
        SetTableValue(185, 255./255, 191./255, 0./255);
        SetTableValue(186, 255./255, 193./255, 0./255);
        SetTableValue(187, 255./255, 195./255, 0./255);
        SetTableValue(188, 255./255, 197./255, 0./255);
        SetTableValue(189, 255./255, 199./255, 0./255);
        SetTableValue(190, 255./255, 201./255, 0./255);
        SetTableValue(191, 255./255, 203./255, 0./255);
        SetTableValue(192, 255./255, 205./255, 0./255);
        SetTableValue(193, 255./255, 206./255, 0./255);
        SetTableValue(194, 255./255, 208./255, 0./255);
        SetTableValue(195, 255./255, 210./255, 0./255);
        SetTableValue(196, 255./255, 212./255, 0./255);
        SetTableValue(197, 255./255, 213./255, 0./255);
        SetTableValue(198, 255./255, 215./255, 0./255);
        SetTableValue(199, 255./255, 217./255, 0./255);
        SetTableValue(200, 255./255, 219./255, 0./255);
        SetTableValue(201, 255./255, 220./255, 0./255);
        SetTableValue(202, 255./255, 222./255, 0./255);
        SetTableValue(203, 255./255, 224./255, 0./255);
        SetTableValue(204, 255./255, 226./255, 0./255);
        SetTableValue(205, 255./255, 228./255, 0./255);
        SetTableValue(206, 255./255, 230./255, 0./255);
        SetTableValue(207, 255./255, 232./255, 0./255);
        SetTableValue(208, 255./255, 234./255, 0./255);
        SetTableValue(209, 255./255, 235./255, 4./255);
        SetTableValue(210, 255./255, 237./255, 8./255);
        SetTableValue(211, 255./255, 239./255, 13./255);
        SetTableValue(212, 255./255, 241./255, 17./255);
        SetTableValue(213, 255./255, 242./255, 21./255);
        SetTableValue(214, 255./255, 244./255, 26./255);
        SetTableValue(215, 255./255, 246./255, 30./255);
        SetTableValue(216, 255./255, 248./255, 35./255);
        SetTableValue(217, 255./255, 248./255, 42./255);
        SetTableValue(218, 255./255, 249./255, 50./255);
        SetTableValue(219, 255./255, 250./255, 58./255);
        SetTableValue(220, 255./255, 251./255, 66./255);
        SetTableValue(221, 255./255, 252./255, 74./255);
        SetTableValue(222, 255./255, 253./255, 82./255);
        SetTableValue(223, 255./255, 254./255, 90./255);
        SetTableValue(224, 255./255, 255./255, 98./255);
        SetTableValue(225, 255./255, 255./255, 105./255);
        SetTableValue(226, 255./255, 255./255, 113./255);
        SetTableValue(227, 255./255, 255./255, 121./255);
        SetTableValue(228, 255./255, 255./255, 129./255);
        SetTableValue(229, 255./255, 255./255, 136./255);
        SetTableValue(230, 255./255, 255./255, 144./255);
        SetTableValue(231, 255./255, 255./255, 152./255);
        SetTableValue(232, 255./255, 255./255, 160./255);
        SetTableValue(233, 255./255, 255./255, 167./255);
        SetTableValue(234, 255./255, 255./255, 175./255);
        SetTableValue(235, 255./255, 255./255, 183./255);
        SetTableValue(236, 255./255, 255./255, 191./255);
        SetTableValue(237, 255./255, 255./255, 199./255);
        SetTableValue(238, 255./255, 255./255, 207./255);
        SetTableValue(239, 255./255, 255./255, 215./255);
        SetTableValue(240, 255./255, 255./255, 223./255);
        SetTableValue(241, 255./255, 255./255, 227./255);
        SetTableValue(242, 255./255, 255./255, 231./255);
        SetTableValue(243, 255./255, 255./255, 235./255);
        SetTableValue(244, 255./255, 255./255, 239./255);
        SetTableValue(245, 255./255, 255./255, 243./255);
        SetTableValue(246, 255./255, 255./255, 247./255);
        SetTableValue(247, 255./255, 255./255, 251./255);
        SetTableValue(248, 255./255, 255./255, 255./255);
        SetTableValue(249, 255./255, 255./255, 255./255);
        SetTableValue(250, 255./255, 255./255, 255./255);
        SetTableValue(251, 255./255, 255./255, 255./255);
        SetTableValue(252, 255./255, 255./255, 255./255);
        SetTableValue(253, 255./255, 255./255, 255./255);
        SetTableValue(254, 255./255, 255./255, 255./255);
        SetTableValue(255, 255./255, 255./255, 255./255);


    } else {
        vtkWarningWithObjectMacro(this, "lookup table not supported: " << type);   
    }

    this->Build();
    this->Modified(); 
}


//! Sets % of table values below threshold to be transparent.
void svkLookupTable::SetAlphaThreshold(double thresholdPercentage) 
{
    this->alphaThresholdPercentage = thresholdPercentage;
    this->ConfigureAlphaThreshold();
}


//
double svkLookupTable::GetAlphaThreshold() 
{
    return this->alphaThresholdPercentage;
}


/*!
 *  Method returns the actual value that it is thresholding on. Not
 *  the percentage but the value within the range above which nothing
 *  will be visible.
 */
double svkLookupTable::GetAlphaThresholdValue() 
{
    int firstVisibleIndex = static_cast<int>( ceil( this->alphaThresholdPercentage * this->GetNumberOfTableValues()) );
    double trueThreshold = ((double)firstVisibleIndex)/ this->GetNumberOfTableValues();
    double thresholdValue = this->GetRange()[0] + (trueThreshold)*(this->GetRange()[1] - this->GetRange()[0]);
    return thresholdValue;
}


//! Resets the alpha value to zero for lut values below threshold percentage and rebuilds the lut. 
void svkLookupTable::ConfigureAlphaThreshold() 
{
    double rgba[4];
    double alphaBelowThreshold = 0.;
    double alphaAboveThreshold = 1.;
    if( this->reverseThreshold ) {
    	alphaBelowThreshold = 1;
    	alphaAboveThreshold = 0;
    }

    for( int i = 0; i < this->GetNumberOfTableValues(); i++ ) {
        this->GetTableValue(i, rgba);
        if( i < this->alphaThresholdPercentage * this->GetNumberOfTableValues() ) {
            rgba[3] = alphaBelowThreshold;
        } else {
            rgba[3] = alphaAboveThreshold;
        }
        this->SetTableValue(i, rgba);
    }
    this->Build(); 
}


/*!
 *
 */
void svkLookupTable::PrintLUT()
{
    for (int i = 0; i < svkLookupTable::NUM_COLORS; i++) {
        cout << "LUT(" << i << ") = " << GetTableValue(i)[0] << " " ;
                                      cout << GetTableValue(i)[1] << " " ;
                                      cout << GetTableValue(i)[2] << " " ;
                                      cout << GetTableValue(i)[3] << " " << endl;
    }

}
