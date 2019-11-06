#ifndef PERFCOMPILER_H_
#define PERFCOMPILER_H_

#include <vector>

#include <asmjit/x86/x86compiler.h>
#include "jitdump.h"


class PerfCompiler : public asmjit::x86::Compiler {
private:
	struct DebugLine {
		const char *file;
		size_t line;

		DebugLine(const char *file, size_t line) : file(file), line(line) {}
	};
	std::vector<DebugLine> debugLines;

	JitDump jd;

public:
	explicit PerfCompiler(asmjit::CodeHolder *code) noexcept : asmjit::x86::Compiler(code) {
		jd.init();
	}
	virtual ~PerfCompiler() {
		jd.close();
	}

	// implicitly attached to latest node
	void attachDebugLine(const char *file_name=__builtin_FILE(), int line_number=__builtin_LINE()){
		debugLines.emplace_back(file_name, line_number);
		// attach to node created last, uses size to have index off by one
		cursor()->setUserDataAsUInt64(debugLines.size());
	}

	void addCodeSegment(const char *fn_name, void *fn, uint64_t code_size){
		jd.addCodeSegment(fn_name, fn, code_size);
	}

	asmjit::Error finalize(){
		asmjit::Error err = runPasses();
		if(err) return err;
		
		asmjit::x86::Assembler a(_code);

		//FIXME: mostly copy & paste from BaseBuilder::serialize(), only handling of InstNode differs
		asmjit::BaseNode *node_ = _firstNode;
		do {
			a.setInlineComment(node_->inlineComment());

			if (node_->isInst()) {
				asmjit::InstNode *node = node_->as<asmjit::InstNode>();

				//FIXME: breaks if user data is used for something else
				// get debug info index from user data on node
				uint64_t dbgindex = node->userDataAsUInt64();
				if(dbgindex != 0){ // check if set
					--dbgindex; // index has offset of 1, to have zero as NULL
					const DebugLine &dl = debugLines[dbgindex];
					jd.addDebugLine(a.offset(), dl.file, dl.line);
				}

				err = a.emitInst(node->baseInst(), node->operands(), node->opCount());
			} else if (node_->isLabel()) {
				if (node_->isConstPool()) {
					asmjit::ConstPoolNode *node = node_->as<asmjit::ConstPoolNode>();
					err = a.embedConstPool(node->label(), node->constPool());
				} else {
					asmjit::LabelNode *node = node_->as<asmjit::LabelNode>();
					err = a.bind(node->label());
				}
			} else if (node_->isAlign()) {
				asmjit::AlignNode *node = node_->as<asmjit::AlignNode>();
				err = a.align(node->alignMode(), node->alignment());
			} else if (node_->isEmbedData()) {
				asmjit::EmbedDataNode *node = node_->as<asmjit::EmbedDataNode>();
				err = a.embed(node->data(), node->size());
			} else if (node_->isEmbedLabel()) {
				asmjit::EmbedLabelNode *node = node_->as<asmjit::EmbedLabelNode>();
				err = a.embedLabel(node->label());
			} else if (node_->isEmbedLabelDelta()) {
				asmjit::EmbedLabelDeltaNode *node = node_->as<asmjit::EmbedLabelDeltaNode>();
				err = a.embedLabelDelta(node->label(), node->baseLabel(), node->dataSize());
			} else if (node_->isSection()) {
				asmjit::SectionNode *node = node_->as<asmjit::SectionNode>();
				err = a.section(_code->sectionById(node->id()));
			} else if (node_->isComment()) {
				asmjit::CommentNode *node = node_->as<asmjit::CommentNode>();
				err = a.comment(node->inlineComment());
			}

			if (err) break;
			node_ = node_->next();
		} while (node_);

		return err;
	}
};

#endif
