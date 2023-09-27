#include "llvm/IR/CFG.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/IRBuilder.h"
#include <vector>
#include <iostream>
#include <map>
#include <set>
#include <random>
using namespace llvm;
std::unique_ptr<Module> M;
#define CAPACITOR 50

std::map<int, int> mp;
std::map<int, std::vector<BasicBlock *>> curr_block;

std::map<int, std::vector<int>> mv;
// path , block Boundry Point
std::map<int, std::vector<int>> BoundaryPoints;

std::set<int> marked;
void Is_path_predicted_to_exceed_capacity(int idx)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(CAPACITOR, 100);

    int percnt = dis(gen);

    std::cout << "Probability to exceed capacitor capacity is : " << percnt << "%\n";
}
int currPath;
// Count instructions in a basic block
unsigned int countInstructions(BasicBlock *BB)
{
    unsigned int Count = 0;
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I)
    {
        ++Count;
    }
    return Count;
}

// Find all blocks in a path and print number of instructions in each block
void findBlocksInPath(std::vector<BasicBlock *> &Path)
{
    if (marked.find(currPath) != marked.end())
    {
        ++currPath;
        return;
    }
    int totalInstructions = 0;
    std::cout << "    Blocks in path:\n";
    for (unsigned int j = 0; j < Path.size(); ++j)
    {
        BasicBlock *BB = Path[j];
        std::cout << "      " << BB->getName().str() << ": " << countInstructions(BB) << " instructions\n";
        totalInstructions += countInstructions(BB);
        mv[currPath].push_back(countInstructions(BB));
    }
    curr_block[currPath] = Path;
    mp[currPath++] = totalInstructions;
}

void findAllPathsHelper(BasicBlock *BB, BasicBlock *TargetBB, std::vector<BasicBlock *> &Path, std::vector<std::vector<BasicBlock *>> &Paths)
{
    if (BB == TargetBB)
    {
        Paths.push_back(Path);
        return;
    }

    for (succ_iterator SI = succ_begin(BB), E = succ_end(BB); SI != E; ++SI)
    {
        BasicBlock *SuccBB = *SI;
        if (std::find(Path.begin(), Path.end(), SuccBB) == Path.end())
        {
            Path.push_back(SuccBB);
            findAllPathsHelper(SuccBB, TargetBB, Path, Paths);
            Path.pop_back();
        }
    }
}

// Find all paths in a CFG from the entry block to a given block
void findAllPaths(BasicBlock *EntryBB, BasicBlock *TargetBB, std::vector<std::vector<BasicBlock *>> &Paths)
{
    std::vector<BasicBlock *> Path;
    Path.push_back(EntryBB);
    findAllPathsHelper(EntryBB, TargetBB, Path, Paths);
}

// Count all paths in a CFG and print instructions in each path
int countPaths(Function &F)
{
    BasicBlock *EntryBB = &F.getEntryBlock();
    std::vector<BasicBlock *> Targets;
    for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
    {
        if (succ_begin(&*BB) == succ_end(&*BB))
        {
            Targets.push_back(&*BB);
        }
    }

    std::vector<std::vector<BasicBlock *>> Paths;
    for (unsigned int i = 0; i < Targets.size(); ++i)
    {
        findAllPaths(EntryBB, Targets[i], Paths);
    }

    std::cout << "Function " << F.getName().str() << " has " << Paths.size() << " paths\n";
    for (unsigned int i = 0; i < Paths.size(); ++i)
    {
        std::cout << "  Path " << i + 1 << ":\n";
        findBlocksInPath(Paths[i]);
    }

    return Paths.size();
}
void SplitPath(int mxP)
{
    marked.insert(mxP);
    std::vector<int> st;
    for (auto e : mv[mxP])
        st.push_back(e);
    int cS = 0;
    int curr = 0;
    for (auto *BB : curr_block[mxP])
    {
        for (auto &Inst : *BB)
        {
            // Do something with Inst, such as printing its name or opcode
            std::cout << curr++ << "\n";
            // errs() << "Instruction name: " << Inst.getName() << "\n";
            // errs() << "Instruction opcode: " << Inst.getOpcode() << "\n";
            if (curr + 1 > mp[mxP] / 2)
            {
                std::cout << "Boundary placed at \n";
                // Create an IRBuilder and set the insertion point to the next instruction
                if (Inst.getNextNode() != NULL)
                {
                    Instruction *NextInst = Inst.getNextNode();
                    IRBuilder<> builder1(NextInst);
                    builder1.SetInsertPoint(NextInst);
                    if (M->getFunction("_Z10printHellov") == NULL)
                        std::cout << "Func not found\n";
                    else
                    {
                        Function *LogFunc = M->getFunction("_Z10printHellov");
                        builder1.CreateCall(LogFunc);
                        // Create the function call instruction
                        Value *RetVal = builder1.CreateCall(LogFunc);

                        // Print the return value
                        errs() << "Return value: " << *RetVal << "\n";
                    }
                    curr = 0;
                }
            }
        }
    }
    // for (int i = 0; i < st.size(); ++i)
    // {
    //     cS += st[i];
    //     if (cS > mp[mxP] / 2)
    //     {
    //         std::cout << "Boundary placed at " << i + 1 << "block in path " << mxP << "\n";
    //         BoundaryPoints[mxP].push_back(i + 1);
    //         cS = 0;
    //     }
    // }
    mp.clear();
    mv.clear();
    currPath = 0;
}

int main()
{
    LLVMContext Context;
    SMDiagnostic Error;
    M = parseIRFile("ms.bc", Error, Context);
    // for (Function &F : *M)
    // {
    //     errs() << "Function name: " << F.getName() << "\n";
    // }
    if (!M)
    {
        std::cerr << "error: failed to load module\n";
        return 1;
    }

    for (Module::iterator F = M->begin(), E = M->end(); F != E; ++F)
    {
        std::cout << "Function " << F->getName().str() << " has " << countPaths(*F) << " paths\n";
    }
    int mxEnergy = INT_MIN;
    int mxP = -1;
    for (auto e : mp)
    {
        std::cout << "Energy of path " << e.first << " "
                  << " : " << e.second << "\n";
        if (e.second > mxEnergy)
        {

            mxEnergy = std::max(mxEnergy, e.second);
            mxP = e.first;
        }
    }
    std::cout << "Max Energy Path : " << mxEnergy << " " << mxP << "\n";
    if (mxEnergy < CAPACITOR)
    {
        for (auto e : BoundaryPoints)
        {
            for (auto ee : e.second)
                std::cout << "Boundary in path " << e.first << "at " << ee << "\n";
        }
        std::cout << "NO Placement Exists\n";
    }
    else
    {
        Is_path_predicted_to_exceed_capacity(mxP);
        SplitPath(mxP);
        main();
    }
    return 0;
}
