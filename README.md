## HomemadeVPN: A miniature networking personal project

---

### Description
This repo contains my work for a virtual L2 switch and client software. The product uses a switch application to 
abstract the functionality of a physical switch, routing ethernet frames digitality rather than through literal
ports. By doing so, it essentially allows two connected remote systems to communicate with one another as if they
were on a local network. Very similar to what a VPN does -- **hence, my own HomemadeVPN!**

All of this work is based heavily on **peiyuanix**'s build-your-own-zerotier repo, and all credit goes to them. I have 
added my own functionality, such as a *gui implementation*, *stylized MAC table printouts*, and various *error handling* 
methods, but the core of the system is based on peiyuanix's work.

**See peiyuanix's repo here -->** https://github.com/peiyuanix/build-your-own-zerotier?tab=readme-ov-file

---

### Goals
- Implement a gui application to build off of switch code and work on full-stack development patterns
- Employ design patterns, error handling, and other development patterns to work on good programming habits
- Get a better grasp of python's socket library and how languages handle TCP/UDP traffic
- Learn about switches and VPNs at a fundamental level by coding my own virtual products of both
- Unify my interests in networking with software development
