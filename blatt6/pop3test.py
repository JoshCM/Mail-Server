import poplib
pop = poplib.POP3("localhost", 8110)
pop.set_debuglevel(1)
pop.user("joendhard")
pop.pass_("geheim")
print(pop.stat(),pop.list(),pop.retr(1))
pop.close()
