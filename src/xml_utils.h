
#ifndef __XML_UTILS_H_
#define __XML_UTILS_H_

// A simple experiment on implementing a syntax highlighting engine which uses the Sublime's syntax defintion

#include <stdint.h>
#include <stdbool.h>

#include <libxml/parser.h>

// common utils for working with libxml

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Supplied with the `node` argument, it returns the node itself if given a element, or the nearest next element that is a sibling of the node or NULL if no element appears after node in the child list. 
 * calling this function with `xmlNodeNextElementSibling(node->next)` is equal of the DOM API's `element.nextElementSibling` getter.
 */
xmlNode *xmlNodeNextElementSibling(xmlNode *node);

char *xmlElementCopyPListString(xmlNode *node);

/**
 * Allocates a string value that contains the pure text of the given `node` matching the behaivor of DOM API's `element.textContent`
 */
char *xmlElementDuplicateTextContent(xmlNode *node);
int xmlNodeTextContent(xmlNode *node, char *buf, size_t bufsz);

/**
 * Returns a reference to the string value enclosed within a `<key>` element. But it fails by returning `NULL` whenever such key element does contains more than one text-node.
 */
const char *xmlElementGetPListKey(xmlNode *node);


bool xmlElementGetPListBoolean(xmlNode *node);

/**
 * Returns a boolean `true` if the given node matches a `<string>` element.
 */
bool xmlElementIsPListString(xmlNode *node);

/**
 * Returns a boolean `true` if the given node matches a `<true>` or `<false>` element.
 */
bool xmlElementIsPListBoolean(xmlNode *node);

/**
 * Returns a boolean `true` if the given node matches a `<key>` element.
 */
bool xmlElementIsPListKey(xmlNode *node);

/**
 * Returns a boolean `true` if the given node matches a `<dict>` element.
 */
bool xmlElementIsPListDict(xmlNode *node);

/**
 * Returns a boolean `true` if the given node matches a `<array>` element.
 */
bool xmlElementIsPListArray(xmlNode *node);

#ifdef __cplusplus
}
#endif

#endif /* __XML_UTILS_H_ */