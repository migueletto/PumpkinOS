typedef struct ChatType ChatType;

ChatType *ChatOpen(char *host, UInt16 port);
void ChatClose(ChatType *chat);

int ChatQuery(ChatType *chat, char *query, Boolean (*response)(char *buf, void *data), void *data);
