%{  
    Copyright © 2016-2017 The Regents of the University of California.
    All Rights Reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    •   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
    •   Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
    •   None of the names of any campus of the University of California, the name
        "The Regents of the University of California," or the names of any of its
        contributors may be used to endorse or promote products derived from this
        software without specific prior written permission.
 
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
    OF SUCH DAMAGE.
%}
function interPlane = compute_sat_interplane_coronal(satPos, nslice)

y = (1:nslice)';
slope1 = (satPos(2,2)-satPos(1,2))/(satPos(2,1)-satPos(1,1));
if slope1 == 0
    interPlane = [y satPos(1,1)*ones(size(y,1),1) satPos(2,1)*ones(size(y,1),1)];
elseif slope1 == inf
    interPlane = [y -5*ones(size(y,1),1) -5*ones(size(y,1),1)];
    s = round(min(satPos(2,2),satPos(1,2)));
    i = round(max(satPos(2,2),satPos(1,2)));
    a = round(min(satPos(1,1),satPos(3,1)));
    p = round(max(satPos(1,1),satPos(3,1)));
    interPlane(s:i,2:3) = repmat([a p],[i-s+1,1]);
else
    % line 1
    x1 = (y-satPos(1,2))/slope1+satPos(1,1);
    
    % line 2
    slope2 = (satPos(3,2)-satPos(2,2))/(satPos(3,1)-satPos(2,1));
    x2 = (y-satPos(2,2))/slope2+satPos(2,1);
    
    % line 3
    slope3 = (satPos(4,2)-satPos(3,2))/(satPos(4,1)-satPos(3,1));
    x3 = (y-satPos(3,2))/slope3+satPos(3,1);
    
    % line 3
    slope4 = (satPos(1,2)-satPos(4,2))/(satPos(1,1)-satPos(4,1));
    x4 = (y-satPos(4,2))/slope4+satPos(4,1);
    X = [x1 x2 x3 x4];
    X = sort(X,2);
    interPlane = [y round(X(:,2:3))];
end

