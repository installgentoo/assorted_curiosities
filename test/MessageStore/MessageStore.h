#pragma once
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <functional>

namespace message_store
{

using std::string;
using std::vector;
using std::map;


struct Message
{
  string sender;
  string recipient;
  string message;
  std::time_t timestamp;
};

struct MessageStore
{
  // stores user names with messages they sent
  typedef map<string, vector<Message>> UserMessageMap;

  UserMessageMap const& userMessages()const { return m_user_messages; }

  bool UserExists(string const&user)const;

  bool RegisterUser(string user);
  bool SendMessage(string sender, string recipient, string message);
  // callback returns true when message should be deleted and false otherwise
  bool RecieveMessages(string recipient, std::function<bool(Message const&)> reception_callback);

private:

  UserMessageMap m_user_messages;
};


// function that prints console interface for interacting with MessageStore
bool ProcessMessaging(MessageStore &message_store);

}
