#include <iostream>

#include "Command.hpp"
#include "commands/NameCommand.hpp"
#include "commands/QuitCommand.hpp"
#include "commands/WhisperCommand.hpp"

Command::Command(Commands command) 
: command(command)
{
}

Message Command::serialize() {
    json j;

    j["type"] = "command";
    j["command"] = enumToString(this->command);

    return j.dump();
}

Command* Command::deserialize(Message message) {
    auto parsed_message = json::parse(message);
    if (parsed_message["type"] != "command") {
        throw "Unexpected message";
    }

    //Insert every command.
    if (parsed_message["command"] == Commands::QUIT) {
        return QuitCommand::deserialize(message);
    }
    if (parsed_message["command"] == Commands::NAME) {
        return NameCommand::deserialize(message);
    }
    if (parsed_message["command"] == Commands::WHISPER) {
        return WhisperCommand::deserialize(message);
    }

    throw "Unknown command";
}

const std::string enumToString(Commands command) {
    switch (command)
    {
    case Commands::NAME: return "name";
    case Commands::QUIT: return "quit";
    case Commands::WHISPER: return "whisper";
    case Commands::JOIN: return "join";
    case Commands::CREATE: return "create";
    case Commands::UNKNOWN: return "unknown";

    default: return "unknown";
    }
}

Commands Command::getCommandType() {
    return this->command;
}