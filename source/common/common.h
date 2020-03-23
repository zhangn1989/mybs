#include <QByteArray>

enum ProtocolType
{
	TEXT = 0,
	PICTURE,
	END,
	MAX
};

struct MYBSProtocol
{
	int length;
	ProtocolType type;
	char data[0];
};