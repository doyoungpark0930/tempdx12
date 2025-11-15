#pragma once

class ModelLoader;
struct aiAnimation;
struct aiNode;
//class Bone;
#include "Bone.h" //나중에 std없애면 이거 없애고 class Bone으로 할수잇음. 이거 지금 std::vector<Bone> m_Bones;여기서 Bone의 사이즈 못구한대서..
#include <unordered_map>
struct AssimpNodeData 
{
    Matrix transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:  
    void OnInit(maxNode* rootNode, ModelLoader* model);

    ~Animation();

    std::vector<Bone>& GetBone() { return m_Bones; }

    inline float GetTicksPerSecond() { return m_TicksPerSecond; }

    inline float GetDuration() { return m_Duration; }

    inline const maxNode* GetRootNode() { return m_RootNode; }

    inline std::unordered_map<std::string, BoneInfo>& GetBoneIDMap()
    {
        return m_BoneInfoMap;
    }

private:
    void InitBones(const maxNode* animation, ModelLoader& model);

    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    maxNode* m_RootNode;
    std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;

    int test = 0;
};