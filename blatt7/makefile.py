#!/usr/bin/env python3

msg = """HELO clientname.de
MAIL FROM:<joghurta@brautweim.de>
RCPT TO:<j.biffel@maildingsi.nl>
DATA
Wie geht es Dir?
Mir geht es gut.
Oma auch.
.
QUIT
""".replace("\n","\r\n")

open("testdatei.txt","w").write(msg)

