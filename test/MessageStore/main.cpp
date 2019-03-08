#include "MessageStore.h"

using namespace message_store;

int main()
{
  MessageStore store;

  while (ProcessMessaging(store))
  { }

  return 0;
}
