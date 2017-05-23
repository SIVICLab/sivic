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
function interPlane = compute_sat_interplane_saggital(satPos, ncol)

x = (1:ncol)';
slope1 = (satPos(2,2)-satPos(1,2))/(satPos(2,1)-satPos(1,1));
if slope1 == inf || slope1 == -inf || isnan(slope1)
    a = min(satPos(2,2),satPos(1,2));
    p = max(satPos(2,2),satPos(1,2));
    interPlane = [x a*ones(size(x,1),1) p*ones(size(x,1),1)];
elseif slope1 == 0
    interPlane = [x -5*ones(size(x,1),1) -5*ones(size(x,1),1)];
    r = round(min(satPos(2,1),satPos(1,1)));
    l = round(max(satPos(2,1),satPos(1,1)));
    if l > 0
        a = round(min(satPos(1,2),satPos(3,2)));
        p = round(max(satPos(1,2),satPos(3,2)));
        interPlane(max(r,1):l,2:3) = repmat([a p],[l-max(r,1)+1,1]);
    end
else
    % line 1
    y1 = (x-satPos(1,1))*slope1+satPos(1,2);
    
    % line 2
    slope2 = (satPos(3,2)-satPos(2,2))/(satPos(3,1)-satPos(2,1));
    y2 = (x-satPos(2,1))*slope2+satPos(2,2);
    
    % line 3
    slope3 = (satPos(4,2)-satPos(3,2))/(satPos(4,1)-satPos(3,1));
    y3 = (x-satPos(3,1))*slope3+satPos(3,2);
    
    % line 3
    slope4 = (satPos(1,2)-satPos(4,2))/(satPos(1,1)-satPos(4,1));
    y4 = (x-satPos(4,1))*slope4+satPos(4,2);
    Y = [y1 y2 y3 y4];
    Y = sort(Y,2);
    interPlane = [x round(Y(:,2:3))];
end

