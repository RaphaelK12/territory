//Dependencies
#include "mandate.h"
#include "../world/world.h"
#include "task.h"
#include "memory.h"
#include "../render/sprite.h"
#include "state.h"
#include "../render/audio.h"
//Link to class
#include "bot.h"


//Constructor
Bot::Bot(int _ID){
  viewDistance = glm::vec3(2);
  memorySize = 25;
  shortermSize = 5;
  trail = false;
  fly = false;
  species = "Human";
  ID = _ID;
  pos = glm::vec3(0);
  home = glm::vec3(0);
}

Bot::Bot(std::string s, bool t, bool f, int forag, int mem, int id, glm::vec3 _pos, glm::vec3 _view, glm::vec3 _home){
  viewDistance = _view;
  forage = forag;
  memorySize = mem;
  shortermSize = 5;
  trail = t;
  fly = f;
  species = s;
  ID = id;
  pos = _pos;
  home = _home;
}

bool Bot::tryInterrupt(State _state){
  interrupt = true;
  return true;
}

/*
Bots make sounds, but so do other things.
So we should actually have a storage vector of sound effects, and process them by the audio guy as they come.
*/

//Per Tick Executor Task
void Bot::executeTask(World &world, Population &population, Audio &audio){
  //Check for interrupt!
  if(interrupt){
    //Reevaluate Mandates, so we can respond
    Task *decide = new Task("Handle Interrupt", ID, &Task::decide);
    current = decide;
    interrupt = false;
  }
  else{
    //Execute Current Task, upon success overwrite.
    if((current->perform)(world, population, audio)){
      Task *decide = new Task("Decide on Action", ID, &Task::decide);
      current = decide;
    }
  }
}

//Merge the Inventories
void Bot::mergeInventory(Inventory _merge){
  //Loop over the stuff that we picked up, and sort it into the inventory
  for(unsigned int i = 0; i < _merge.size(); i++){
    bool found = false;
    for(unsigned int j = 0; j < inventory.size(); j++){
      if(_merge[i]._type == inventory[j]._type){
        inventory[j].quantity += _merge[i].quantity;
        found = true;
      }
    }
    //We haven't found the item in the inventory, so add it to the back
    if(!found){
      inventory.push_back(_merge[i]);
    }
  }
}

//Find any memories matching a query and overwrite them
void Bot::updateMemory(Memory &query, bool all, Memory &memory){
  //Loop through all existing Memories
  for(unsigned int i = 0; i < memories.size(); i++){
    //If all matches are required and we have all matches
    if(all && (memories[i] == query)){
      //We have a memory that needs to be updated
      memories[i] = memory;
      continue;
    }
    //If not all matches are required and any query elements are contained
    else if(!all && (memories[i] || query)){
      memories[i] = memory;
      continue;
    }
  }
}

std::deque<Memory> Bot::recallMemories(Memory &query, bool all){
  //When we query a memory, it is important if it should fit any criteria or all criteria.
  //New Memory
  std::deque<Memory> recalled;
  //Loop through memories
  for(unsigned int i = 0; i < memories.size(); i++){
    //If all matches are required and we have all matches
    if(all && (memories[i] == query)){
      recalled.push_back(memories[i]);
      memories[i].recallScore++;
      continue;
    }
    //If not all matches are required and any query elements are contained
    else if(!all && (memories[i] || query)){
      recalled.push_back(memories[i]);
      memories[i].recallScore++;
      continue;
    }
  }
  return recalled;
}

void Bot::addMemory(World world, glm::vec3 _pos){
  //Memory already exists, so overwrite relevant portions.
  for(unsigned int i = 0; i < memories.size(); i++){
    //We already have a memory at that location!
    if(memories[i].state.pos == _pos){
      //Update the Information at that position
      memories[i].state.block = world.getBlock(_pos);
      memories[i].state.task = task;

      //Check if we should remove this memory.
      if(memories[i].state.block == BLOCK_AIR){
        memories.erase(memories.begin()+i);
      }
      return;
    }
  }

  Memory memory;
  memory.state.pos = _pos;
  memory.state.block = world.getBlock(memory.state.pos);
  if(memory.state.block == BLOCK_AIR) return;
  if(memory.state.block == BLOCK_STONE) return;
  if(memory.state.block == BLOCK_DIRT) return;
  if(memory.state.block == BLOCK_GRASS) return;
  memory.state.task = task;
  memory.state.reachable = true;

  //Now shuffle the memories around.
  for(unsigned int i = 1; i < memories.size(); i++){
    //Check If A Memory Exists at the location
    if(memories[i].recallScore > memories[i-1].recallScore){
      Memory oldMemory = memories[i-1];
      memories[i-1] = memories[i];
      memories[i] = oldMemory;
    }
  }

  //Add new Memory
  memories.push_front(memory);

  //Remove the Last Element if it is too much
  if(memories.size() > memorySize){
    memories.pop_back();
  }
}

void Bot::addSound(State _state){
  //Create the Auditory Information
  Memory sound;
  sound.state = _state;

  //Add it to the front
  shorterm.push_front(sound);

  //Clip anything hanging off the back
  if(shorterm.size() > shortermSize){
    shorterm.pop_back();
  }
}

void Bot::setupSprite(){
  //Load the Sprite Thing
  sprite.loadImage("hunterfull.png");
  sprite.setupBuffer(false);
  sprite.resetModel();
}
