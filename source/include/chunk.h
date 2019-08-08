enum BiomeType{
  BIOME_VOID = 0,
  BIOME_DESERT = 1,
  BIOME_FOREST = 2,
};

class Chunk{
public:
  //Position information and size information
  glm::vec3 pos;
  int size;
  bool hasModel = false;
  int modelID;
  BiomeType biome;

  //Data Storage Member
  Octree data;  //Raw Data
  bool updated = false;  //Has the data been updated?
  bool refreshModel = true;

  //Get Color Data
  glm::vec4 getColorByID(BlockType _type);

  //Chunk Manipulation Functions
  void fillVolume(glm::vec3 start, glm::vec3 end, BlockType _type);
  void hollowBox(glm::vec3 start, glm::vec3 end, BlockType _type);
};
