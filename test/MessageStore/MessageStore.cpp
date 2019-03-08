#include "MessageStore.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace message_store;
using std::move;

/// macros have a benefit of being separate color from keywords, which does improve visibility for constants
/// not usable for existing codebase, but i am imagining that i'm setting codesyle here
/// as you might notice further on i do like my constants

#define VAL const auto


bool MessageStore::UserExists(const std::string &user)const
{
  return m_user_messages.find(user) != m_user_messages.cend();
}

bool MessageStore::RegisterUser(string user)
{
  if (this->UserExists(user))
    return false;

  m_user_messages.emplace(move(user), vector<Message>{ });
  return true;
}

bool MessageStore::SendMessage(string sender, string recipient, string message)
{
  if (!this->UserExists(recipient))
    return false;

  auto found_sender = m_user_messages.find(sender);
  if (found_sender == m_user_messages.cend())
    return false;

  auto &sender_messages = found_sender->second;
  sender_messages.emplace_back(Message{ move(sender), move(recipient), move(message), std::time(nullptr) });
  return true;
}

bool MessageStore::RecieveMessages(string recipient, std::function<bool(Message const&)> reception_callback)
{
  VAL found_recipient = m_user_messages.find(recipient);
  if (found_recipient == m_user_messages.cend())
    return false;

  for (auto &sender: m_user_messages)
  {
    auto &sender_messages = sender.second;
    sender_messages.erase(std::remove_if (sender_messages.begin(), sender_messages.end(), reception_callback), sender_messages.cend());
  }

  return true;
}


#define PRINTLN(text) { std::cout << text << "\n"; }


bool message_store::ProcessMessaging(MessageStore &message_store)
{
  static VAL console_wipe = string(80, '\n');

  // clear screen
  PRINTLN(console_wipe);
  // show options
  PRINTLN("Please select an option:");
  PRINTLN("1. Create User");
  PRINTLN("2. Send Message");
  PRINTLN("3. Receive All Messages For User");
  PRINTLN("4. Report All messages");
  PRINTLN("5. Quit");
  PRINTLN("");

  VAL selected_option = []{
    string in;
    std::getline(std::cin, in);

    try{ return std::stoi(in); } catch(...) { }
    return -1;
  }();

  VAL readline = [](VAL &desc){
    std::cout << desc;
    string str;
    std::getline(std::cin, str);
    PRINTLN("");
    return str;
  };

  switch(selected_option)
  {
    case 1:
    {
      VAL name = readline("Please enter name: ");

      if (!message_store.RegisterUser(name))
      {
        PRINTLN("ERROR: User '" << name << "' already exists!");
        break;
      }

      PRINTLN("User '" << name << "' added!");
    }
    break;

    case 2:
    {
      string from = readline("From: ");
      if (!message_store.UserExists(from))
      {
        PRINTLN("ERROR: Sender'" << from << "' doesn't exist!");
        break;
      }

      string to = readline("To: ");
      if (!message_store.UserExists(to))
      {
        PRINTLN("ERROR: Recipient '" << to << "' doesn't exist!");
        break;
      }

      if (!message_store.SendMessage(move(from), move(to), readline("Message: ")))
        break;

      PRINTLN("Message Sent!");
    }
    break;

      using std::cout; using std::endl;

    case 3:
    {
      string user = readline("Enter name of user to receive all messages for: ");
      if (!message_store.UserExists(user))
      {
        PRINTLN("ERROR: Recipient '" << user << "' doesn't exist!");
        break;
      }

      PRINTLN("");
      PRINTLN("===== BEGIN MESSAGES =====");

      auto message_number = 1;

      VAL callback = [&](Message const&msg){
        if (msg.recipient != user)
          return false;

        PRINTLN("Message "  << message_number++);
        PRINTLN("From: "    << msg.sender);
        PRINTLN("Content: " << msg.message);
        PRINTLN("");
        return true;
      };

      message_store.RecieveMessages(user, callback);

      PRINTLN("");
      PRINTLN("===== END MESSAGES =====");
    }
    break;

    case 4:
    {
      for (VAL &sender: message_store.userMessages())
      {
        VAL &messages = sender.second;

        PRINTLN("Sender '" << sender.first << "', (" << messages.size() << " messages):");

        VAL message_its = [&]{
          vector<vector<Message>::const_iterator> its;
          its.reserve(messages.size());

          for (auto i=messages.cbegin(); i!=messages.cend(); ++i)
            its.emplace_back(i);

          std::sort(its.begin(), its.end(), [](VAL &l, VAL &r){ return l->recipient != r->recipient ? l->recipient < r->recipient : l->timestamp < r->timestamp; });
          return its;
        }();

        for (VAL &message: message_its)
        {
          PRINTLN("    " << std::put_time(std::gmtime(&message->timestamp), "%c %Z") << ", recipient '" << message->recipient << "'");
        }
      }
    }
    break;

    case 5:
    {
      PRINTLN("Quitting!");
      std::cout<<std::flush;

      return false;
    }

    default:
    {
      PRINTLN("Invalid Option Selected");
    }
  }

  PRINTLN("");
  readline("Enter any key and press return to continue.....");

  return true;
}
