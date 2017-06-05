#ifndef TL_TREE_HPP
#define TL_TREE_HPP 1

namespace tl {

struct TreeNodeBase {
	TreeNodeBase *left_child, **prev_next, *right_sibling;
	static TreeNodeBase null;

	TreeNodeBase()
		: left_child(&null)
		, prev_next(0)
		, right_sibling(&null) {
	}

	void unlink() {
		this->right_sibling->prev_next = this->prev_next;
		*this->prev_next = this->right_sibling;
	}

	void link_left_child(TreeNodeBase* child) {
		child->prev_next = &this->left_child;
		child->right_sibling = this->left_child;
		this->left_child->prev_next = &child->right_sibling;
		this->left_child = child;
	}

	void link_sibling_after(TreeNodeBase* child) {
		child->prev_next = &this->right_sibling;
		child->right_sibling = this->right_sibling;
		this->right_sibling->prev_next = &child->right_sibling;
		this->right_sibling = child;
	}

	void link_sibling_before(TreeNodeBase* child) {
		child->prev_next = this->prev_next;
		child->right_sibling = this;
		*this->prev_next = child;
		this->prev_next = &child->right_sibling;
	}
};

template<typename DerivedT>
struct TreeNode : protected TreeNodeBase {
	using TreeNodeBase::unlink;

	struct ChildIterator {
		TreeNodeBase* cur;

		ChildIterator(TreeNodeBase* cur_init) : cur(cur_init) {
		}

		ChildIterator& operator++() {
			this->cur = this->cur->right_sibling;
			return *this;
		}

		DerivedT& operator*() const {
			return *(DerivedT *)cur;
		}

		DerivedT* operator->() const {
			return (DerivedT *)cur;
		}

		bool operator==(ChildIterator const& other) const {
			return this->cur == other.cur;
		}

		bool operator!=(ChildIterator const& other) const {
			return this->cur != other.cur;
		}
	};

	struct ChildRange {
		TreeNodeBase** prev_next;

		ChildRange(TreeNode& node_init)
			: prev_next(&node_init.left_child) {
		}

		ChildIterator begin() {
			return ChildIterator(*this->prev_next);
		}

		ChildIterator end() {
			return ChildIterator(&TreeNodeBase::null);
		}

		bool has_more() const {
			return *prev_next != &TreeNodeBase::null;
		}

		DerivedT* next() {
			DerivedT* v = (DerivedT *)*prev_next;
			prev_next = &v->right_sibling;
			return v;
		}

		void link_sibling_after(DerivedT* child) {
			child->prev_next = prev_next;
			child->right_sibling = *prev_next;
			*prev_next = child;
			prev_next = &child->right_sibling;
		}
	};

	ChildRange children() {
		return ChildRange(*this);
	}

	void link_left_child(DerivedT* child) {
		this->TreeNodeBase::link_left_child(child);
	}

	void link_sibling_after(DerivedT* child) {
		this->TreeNodeBase::link_sibling_after(child);
	}

	void link_sibling_before(DerivedT* child) {
		this->TreeNodeBase::link_sibling_before(child);
	}
};

}

#endif // TL_TREE_HPP
