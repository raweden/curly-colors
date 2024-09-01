
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml_utils.h"

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void
print_element_names(xmlNode * a_node)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("node type: Element, name: %s\n", cur_node->name);
        }

        print_element_names(cur_node->children);
    }
}

// Sublime Snippet

void *
sl_xml_parse_snippet(xmlDoc *doc, int *err)
{
    xmlNode *root_element = NULL;
    xmlNode *dictElem, *key_child, *val_child;
    struct tm_syntax_header *grammar;
    uint32_t keysz, keysym;
    int count;

    grammar = NULL;

    // Get the root element node
    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL || root_element->type != XML_ELEMENT_NODE || root_element->name == NULL || strcmp((char *)root_element->name, "snippet") != 0) {
        return NULL;
    }

    print_element_names(root_element->children);

    return NULL;
}


void
sl_snippet_parse(const char *filepath)
{
    xmlDoc *doc = NULL;
    void *def;
    int err;
    LIBXML_TEST_VERSION

    // parse the file and get the DOM
    doc = xmlReadFile(filepath, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filepath);
        return;
    }

    def = sl_xml_parse_snippet(doc, &err);

    // free the document
    xmlFreeDoc(doc);

    // Free the global variables that may have been allocated by the parser.
    xmlCleanupParser();

    return;
}

// TextMate snippet

void *
tm_xml_parse_snippet(xmlDoc *doc, int *err)
{
    xmlNode *root_element = NULL;
    xmlNode *dictElem, *key_child, *val_child;
    struct tm_syntax_header *grammar;
    uint32_t keysz, keysym;
    int count;

    grammar = NULL;

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL || root_element->type != XML_ELEMENT_NODE || root_element->name == NULL || strcmp((char *)root_element->name, "plist") != 0) {
        return NULL;
    }

    dictElem = xmlNodeNextElementSibling(root_element->children);

    if (dictElem == NULL || dictElem->type != XML_ELEMENT_NODE || dictElem->name == NULL || strcmp((char *)dictElem->name, "dict") != 0) {
        return NULL;
    }

    print_element_names(root_element->children);

    return NULL;
}


void
tm_snippet_parse(const char *filepath)
{
    xmlDoc *doc = NULL;
    void *def;
    int err;
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filepath, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filepath);
        return;
    }

    def = tm_xml_parse_snippet(doc, &err);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return;
}

// Theme parsing

void *
tm_xml_parse_theme(xmlDoc *doc, int *err)
{
    xmlNode *root_element = NULL;
    xmlNode *dict_child, *key_child, *val_child;
    struct tm_syntax_header *grammar;
    char *key, *val;
    uint32_t keysz, keysym;
    int count;

    grammar = NULL;

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    if (root_element == NULL || root_element->type != XML_ELEMENT_NODE || root_element->name == NULL || strcmp((char *)root_element->name, "plist") != 0) {
        return NULL;
    }

    dict_child = xmlNodeNextElementSibling(root_element->children);

    if (dict_child == NULL || dict_child->type != XML_ELEMENT_NODE || dict_child->name == NULL || strcmp((char *)dict_child->name, "dict") != 0) {
        return NULL;
    }

    key_child = xmlNodeNextElementSibling(dict_child->children);

    while (key_child != NULL) {
        val_child = xmlNodeNextElementSibling(key_child->next);

        if (xmlElementIsPListString(val_child)) {
            key = (char *)xmlElementGetPListKey(key_child);
            val = xmlElementDuplicateTextContent(val_child);
            printf("'%s': '%s'\n", key, val);
            free(val);
        }
        
        key_child = xmlNodeNextElementSibling(val_child->next);
    }


    print_element_names(root_element->children);

    return NULL;
}

void
tm_theme_parse(const char *filepath)
{
    xmlDoc *doc = NULL;
    void *def;
    int err;
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(filepath, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filepath);
        return;
    }

    def = tm_xml_parse_theme(doc, &err);

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return;
}