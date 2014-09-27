#ifndef LYRICA_H
#define LYRICA_H

class Lyrica
{
public:
	Lyrica(char *_nick, char * _usr);
	virtual ~Lyrica();

//	bool setup;

	int start();
	bool charSearch(char *toSearch, char *searchFor);

private:
	int s;
	int iResult;

	char *nick;
	char *usr;

	bool isConnected(char *buf);
	char * timeNow();
	bool sendData(char *msg);
	void sendPong(char *buf);
	void msgHandle(char *buf);
	void sendCmdString(char *cmd, char *usrinput);
};

#endif /* LYRICA_H */