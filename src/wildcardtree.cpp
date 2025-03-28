// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "wildcardtree.h"

#include <stack>

WildcardTreeNode* WildcardTreeNode::getChild(char ch)
{
	auto it = children.find(ch);
	if (it == children.end()) {
		return nullptr;
	}
	return &it->second;
}

const WildcardTreeNode* WildcardTreeNode::getChild(char ch) const
{
	auto it = children.find(ch);
	if (it == children.end()) {
		return nullptr;
	}
	return &it->second;
}

WildcardTreeNode* WildcardTreeNode::addChild(char ch, bool breakpoint)
{
	WildcardTreeNode* child = getChild(ch);
	if (child) {
		if (breakpoint && !child->breakpoint) {
			child->breakpoint = true;
		}
	} else {
		auto pair =
		    children.emplace(std::piecewise_construct, std::forward_as_tuple(ch), std::forward_as_tuple(breakpoint));
		child = &pair.first->second;
	}
	return child;
}

void WildcardTreeNode::insert(std::string_view str)
{
	WildcardTreeNode* cur = this;

	size_t length = str.length() - 1;
	for (size_t pos = 0; pos < length; ++pos) {
		cur = cur->addChild(str[pos], false);
	}

	cur->addChild(str[length], true);
}

void WildcardTreeNode::remove(std::string_view str)
{
	WildcardTreeNode* cur = this;

	std::stack<WildcardTreeNode*> path;
	path.push(cur);
	size_t len = str.length();
	for (size_t pos = 0; pos < len; ++pos) {
		cur = cur->getChild(str[pos]);
		if (!cur) {
			return;
		}
		path.push(cur);
	}

	cur->breakpoint = false;

	do {
		cur = path.top();
		path.pop();

		if (!cur->children.empty() || cur->breakpoint || path.empty()) {
			break;
		}

		cur = path.top();

		auto it = cur->children.find(str[--len]);
		if (it != cur->children.end()) {
			cur->children.erase(it);
		}
	} while (true);
}

ReturnValue WildcardTreeNode::findOne(std::string_view query, std::string& result) const
{
	const WildcardTreeNode* cur = this;
	for (auto pos : query) {
		cur = cur->getChild(pos);
		if (!cur) {
			return RETURNVALUE_PLAYERWITHTHISNAMEISNOTONLINE;
		}
	}

	result = query;

	do {
		size_t size = cur->children.size();
		if (size == 0) {
			return RETURNVALUE_NOERROR;
		} else if (size > 1 || cur->breakpoint) {
			return RETURNVALUE_NAMEISTOOAMBIGUOUS;
		}

		auto it = cur->children.begin();
		result += it->first;
		cur = &it->second;
	} while (true);
}

void WildcardTreeNode::search(const std::string& query, std::vector<std::string>& result) const
{
	const WildcardTreeNode* cur = this;
	
	// Navigate to the node that represents the query prefix
	for (char c : query) {
		cur = cur->getChild(c);
		if (!cur) {
			return; // No matches found
		}
	}
	
	// If we reached a breakpoint, add the query itself as a result
	if (cur->breakpoint) {
		result.push_back(query);
	}
	
	// Helper function to recursively collect all matches
	std::function<void(const WildcardTreeNode*, std::string)> collectMatches = 
		[&collectMatches, &result](const WildcardTreeNode* node, std::string current) {
			if (node->breakpoint) {
				result.push_back(current);
			}
			
			for (const auto& pair : node->children) {
				collectMatches(&pair.second, current + pair.first);
			}
		};
	
	// Collect all matches that start with the query
	for (const auto& pair : cur->children) {
		collectMatches(&pair.second, query + pair.first);
	}
}

bool WildcardTreeNode::contains(std::string_view str) const
{
	const WildcardTreeNode* cur = this;
	
	// Navigate through the tree following the characters in str
	for (char c : str) {
		cur = cur->getChild(c);
		if (!cur) {
			return false; // Character not found in the tree
		}
	}
	
	// Check if we reached a valid endpoint (breakpoint)
	return cur->breakpoint;
}
