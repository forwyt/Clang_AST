
#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"


using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;

namespace JasonPlugin{
    class JasonMatchFinderCallback:public MatchFinder::MatchCallback{
    private:
        CompilerInstance &CI;
        bool isUserSourceFile(const string filename){
            if (filename.empty()) {
                return false;
            }
            if (filename.find("/Applications/Xcode.app/") == 0) {
                return false;
            }
            return true;
        }
        bool isShouldUseCopy(const string typeString){
            if (typeString.find("NSString") != string::npos || typeString.find("NSArray") != string::npos ||
                typeString.find("NSDictionary") != string::npos) {
                return true;
            }
            return false;
        }
    public:
        JasonMatchFinderCallback(CompilerInstance &CI):CI(CI){}
        void run (const MatchFinder::MatchResult &Result){
            //解析 获取节点
            const ObjCPropertyDecl * propertyDecl = Result.Nodes.getNodeAs<ObjCPropertyDecl>("ObjCPropertyDecl");
            
            string filename = CI.getSourceManager().getFilename(propertyDecl->getSourceRange().getBegin()).str();
            
            if (propertyDecl && isUserSourceFile(filename)) {
                //属性值
                string type = propertyDecl->getType().getAsString();
                //节点的描述信息
                ObjCPropertyAttribute::Kind obAttributeKind =  propertyDecl->getPropertyAttributes();
                if(isShouldUseCopy(type) && !(obAttributeKind & ObjCPropertyAttribute::Kind::kind_copy)){
                    cout<<type<<"may be user copy" <<endl;
                    DiagnosticsEngine &diagno = CI.getDiagnostics();
                    diagno.Report(propertyDecl->getBeginLoc(), diagno.getCustomDiagID(DiagnosticsEngine::Error, "Attribute 可以修改下啦😈👀 👀"));
                }
            }
        }
    };
    class JasonASTConsumer: public ASTConsumer{
    private:
        MatchFinder matcher;
        JasonMatchFinderCallback callback;
        
    public:
        JasonASTConsumer(CompilerInstance &CI):callback(CI){
            matcher.addMatcher(objcPropertyDecl().bind("ObjCPropertyDecl"), &callback);
        }
            
        virtual void HandleTranslationUnit(ASTContext &Ctx){
            cout <<"解析结束啊" <<endl;
            matcher.matchAST(Ctx);

        }
    };
    class JasonASTAction: public PluginASTAction{
    public:
        bool ParseArgs(const CompilerInstance &CI,
                               const std::vector<std::string> &arg){
            return true;
        }
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                      StringRef InFile){
            return unique_ptr<JasonASTConsumer>(new JasonASTConsumer(CI));
        }
        
        
    };
}



static FrontendPluginRegistry::Add<JasonPlugin::JasonASTAction> X("JasonPlugin","This is first jasonphd write plugin demo");
