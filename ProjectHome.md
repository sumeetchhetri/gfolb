## Introduction ##
<font size='5' face='Georgia, Arial'>
A generic implementation of a Load Balancer and Fail over mechanism<br>
</font>
## Features ##
<font size='4' face='Georgia, Arial'>
<ul><li>Multi protocol support<br>
</li><li>Any Text based(HTTP) or Binary protocol(SMPP) server can be supported<br>
</li><li>Supports 3 modes of operation<br>
<ol><li><b>Fail Over</b> (If one goes down transfer load to the other)<br>
</li><li><b>Load Balancer</b> (distribute equal load among the pool of servers)<br>
</li><li><b>Online Routing</b> (using the admin port fire online routing commands)<br>
</li></ol></li><li>Good performance (Code is written in C++)<br>
</li><li>SSL support present<br>
</li><li>Persistent or Non-persistent connections to the pool supported<br>
</font>