---
layout: page
title: Forest Documentation and Tutorial
permalink: /forest/
home-icon: tree
tab: Forest
---

<style>
    svg {
        display: block;
        margin-left: auto;
        margin-right: auto;
       }

    .svg-img {
        display: block;
        margin-left: auto;
        margin-right: auto;
       }
</style>

{% assign node_sz=100 %}
{% assign node_half_sz=node_sz | divided_by: 2 %}
{% assign node_radius=node_half_sz %}
{% assign node_font_size=36 %}
{% assign small_font_size=24 %}
{% assign color_disabled="grey" %}

<svg width='0' height='0'>
    <defs>
        <marker id="arrowhead" markerWidth="6" markerHeight="5" refX="5" refY="2.5" orient="auto">
            <polygon points="0 0, 6 2.5, 0 5" />
        </marker>
        <g id='root'>
            <rect width='{{node_sz}}' height='{{node_sz}}' x='-{{node_half_sz}}' y='-{{node_half_sz}}' fill='white' stroke='darkred' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" dominant-baseline="central">R</text>
        </g>
        <g id='node'>
            <circle r='{{node_radius}}' fill='white' stroke='blue' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" dominant-baseline="central">N<tspan dy="15" font-size=".7em"></tspan></text>
        </g>
        <g id='parent'>
            <circle r='{{node_radius}}' fill='white' stroke='{{color_disabled}}' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" fill='{{color_disabled}}' dominant-baseline="central">P<tspan dy="15" font-size=".7em"></tspan></text>
        </g>
        <g id='sib_prior'>
            <circle r='{{node_radius}}' fill='white' stroke='{{color_disabled}}' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" fill='{{color_disabled}}' dominant-baseline="central">S<tspan dy="15" font-size=".7em">prior</tspan></text>
        </g>
        <g id='sib_next'>
            <circle r='{{node_radius}}' fill='white' stroke='{{color_disabled}}' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" fill='{{color_disabled}}' dominant-baseline="central">S<tspan dy="15" font-size=".7em">next</tspan></text>
        </g>
        <g id='child_first'>
            <circle r='{{node_radius}}' fill='white' stroke='{{color_disabled}}' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" fill='{{color_disabled}}' dominant-baseline="central">C<tspan dy="15" font-size=".7em">first</tspan></text>
        </g>
        <g id='child_last'>
            <circle r='{{node_radius}}' fill='white' stroke='{{color_disabled}}' stroke-width='5'/>
            <text font-size='{{node_font_size}}' text-anchor="middle" fill='{{color_disabled}}' dominant-baseline="central">C<tspan dy="15" font-size=".7em">last</tspan></text>
        </g>
        <g id='edge_lo_ti'>
            <path d='M -25 50 Q -75 125 0 125 Q 75 125 25 50' fill='none' stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_li'>
            <line x1="-100" y1="-100" x2="-40" y2="-40" stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_lo'>
            <line x1="-40" y1="40" x2="-100" y2="100" stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_ti'>
            <line x1="100" y1="100" x2="40" y2="40" stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_to'>
            <line x1="40" y1="-40" x2="100" y2="-100" stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_lo_li'>
            <path d='M 0 50 Q -50 100 0 150' fill='none' stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_lo_li_2'>
            <path d='M 100 50 Q -50 100 0 150' fill='none' stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_to_li'>
            <path d='M 50 0 Q 100 -50 150 0' fill='none' stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_to_ti'>
            <path d='M 50 150 Q 100 100 50 50' fill='none' stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
        <g id='edge_to_ti_2'>
            <path d='M 50 150 Q 100 100 -50 50' fill='none' stroke='black' stroke-width='5' marker-end="url(#arrowhead)"/>?
        </g>
    </defs>
</svg>

* _TOC stub_
{:toc}

A forest is a hierarchical, node-based data structure. This document serves to cover the high-level concepts of a forest, `adobe::forest<T>` implementation details, as well as examples of frequent patterns.

## Concepts

### Terminology

$N$: An exemplar node

$P$: The parent node of $N$

$C$: A child node of $N$. Child nodes may be subscripted for clarity, e.g., $C_{first}$ and $C_{last}$, etc.

$S$: A sibling node of $N$. Sibling nodes may be subscripted for clarity, e.g., $S_{prev}$ and $S_{next}$, etc.

$R$: A root node

$I$: A forest (fullorder) iterator

## Nodes

Forest's fundamental element type is the node. They are heap-allocated by the forest as necessary for storing a value. Each node has exactly one parent and zero or more children. In this document we will draw nodes as circles:

<img class='svg-img' src='{{site.baseurl}}/svg/node.svg'/>

## The Root Node

Every forest has a root node, which is not a node used to store values in the forest. Rather, its primary purpose is as the anchor to which all top-level nodes in the forest are attached. In this document we will draw the root node as a rectangle:

<img class='svg-img' src='{{site.baseurl}}/svg/root.svg'/>

## Edges

Forest nodes are connected to one another with edges. For every node in the forest, there are exactly two edges that lead away from the node, and exactly two that lead towards the node. In this document edges will be drawn as arrows pointed in the direction of forward travel:

<svg width='125' height='125' viewBox='0 0 250 250'>
    <use xlink:href='#edge_li' x='125' y = '125'/>
    <use xlink:href='#edge_lo' x='125' y = '125'/>
    <use xlink:href='#edge_ti' x='125' y = '125'/>
    <use xlink:href='#edge_to' x='125' y = '125'/>
    <use xlink:href='#node' x='125' y='125'/>
</svg>

The two left edges are known as **leading edges**, and the two right edges are known as **trailing edges**:

<svg width='175' height='125' viewBox='0 0 350 250'>
    <use xlink:href='#edge_li' x='175' y = '125'/>
    <use xlink:href='#edge_lo' x='175' y = '125'/>
    <use xlink:href='#edge_ti' x='175' y = '125'/>
    <use xlink:href='#edge_to' x='175' y = '125'/>
    <use xlink:href='#node' x='175' y='125'/>
    <line x1="175" y1="25" x2="175" y2="225" stroke='black' stroke-width='5' stroke-dasharray='10'/>?
    <text font-size='{{small_font_size}}' x='25' y='125' dominant-baseline="central">leading</text>
    <text font-size='{{small_font_size}}' x='245' y='125' dominant-baseline="central">trailing</text>
</svg>

The two edges that point to $N$ are known as **in edges** and the edges that point away from $N$ are **out edges**:

<svg width='150' height='150' viewBox='0 0 300 300'>
    <use xlink:href='#edge_li' x='150' y = '150'/>
    <use xlink:href='#edge_lo' x='150' y = '150'/>
    <use xlink:href='#edge_ti' x='150' y = '150'/>
    <use xlink:href='#edge_to' x='150' y = '150'/>
    <use xlink:href='#node' x='150' y='150'/>
    <text font-size='{{small_font_size}}' x='25' y='30' dominant-baseline="central">in</text>
    <text font-size='{{small_font_size}}' x='10' y='260' dominant-baseline="central">out</text>
    <text font-size='{{small_font_size}}' x='250' y='260' dominant-baseline="central">in</text>
    <text font-size='{{small_font_size}}' x='250' y='30' dominant-baseline="central">out</text>
</svg>

It is worth noting that the terms "in" and "out" are relative to $N$. In other words, an in edge for $N$ will also be an out edge for some other node.

### Leading In Edge

Visually, the northwest edge is the **leading in edge**. It originates (is an out edge) from one of two possible nodes:

- If $N$ is the first child of its parent node $P$, this edge is the leading out edge of $P$:

<img class='svg-img' src='{{site.baseurl}}/svg/li_child.svg'/>

- Otherwise, this edge is the trailing edge of $N$'s prior sibling $S_{prior}$:

<img class='svg-img' src='{{site.baseurl}}/svg/li_sibling.svg'/>

### Leading Out Edge

The southwest edge is the leading out edge. It terminates (is an in edge) at one of two possible nodes:

- If $N$ is a parent, this edge is the leading edge of the first child of $N$, $C_{first}$:

<img class='svg-img' src='{{site.baseurl}}/svg/lo_parent.svg'/>

- Otherwise, this edge is the trailing edge of $N$ itself:

<img class='svg-img' src='{{site.baseurl}}/svg/lo_self.svg'/>

### Trailing In Edge

The southeast edge is the trailing in edge. It terminates (is an in edge) at one of two possible nodes:

- If $N$ is a parent, it comes from the trailing edge of $N$'s last child, $C_{last}$:

<img class='svg-img' src='{{site.baseurl}}/svg/ti_parent.svg'/>

- Otherwise, it comes from the leading edge of $N$ itself:

<img class='svg-img' src='{{site.baseurl}}/svg/ti_self.svg'/>

### Trailing Out Edge

The northeast edge is the trailing out edge. It points to one of two related nodes:

- If $N$ is the last child of $P$, this edge points to the trailing edge of $P$:

<img class='svg-img' src='{{site.baseurl}}/svg/to_child.svg'/>

- Otherwise, this edge points to the leading edge of $N$'s next sibling:

<img class='svg-img' src='{{site.baseurl}}/svg/to_sibling.svg'/>

## Example Forests

With the fundamental building blocks in place, here is how they combine to form some of the basic relationships in a forest. Note that (with the exception of the root node) every node in the forest maintains four relationships with the nodes around it.

### Empty

<img class='svg-img' src='{{site.baseurl}}/svg/empty.svg'/>

Given a forest that contains only the root node, `forest<T>::empty()` will return `true`. Otherwise, it will return `false`.

The root does have a trailing-out edge which always points to the root leading-in edge. This creates a unique and detectible termination case, which is a useful property. For example, the end iterator for a preorder traversal (which always points to a leading edge) is this trailing-out to leading-in edge of the root.

### One Node

<img class='svg-img' src='{{site.baseurl}}/svg/one_node.svg'/>

### Two Sibling Nodes

<img class='svg-img' src='{{site.baseurl}}/svg/two_nodes.svg'/>

### Many Nodes and Levels

<img class='svg-img' src='{{site.baseurl}}/svg/sample.svg'/>

## Iterators

Iterators are comprised of two pieces of data:

- The node ($N$) to which they point
- Whether they are on the leading ($L$) or trailing ($T$) edge of the node

In this documentation, iterators will be described by this pair as `{node, edge}`.

### Examples

- `{P, L}` is an iterator that points to node $P$ on the leading edge:

<svg width='112.5' height='112.5' viewBox='0 0 225 225'>
    <use xlink:href='#edge_li' x='150' y = '150'/>
    <use xlink:href='#parent' x='150' y='150'/>
    <text font-size='{{small_font_size}}' x='25' y='30' dominant-baseline="central">{P, L}</text>
</svg>

- `{N, T}` is an iterator that points to node $N$ on the trailing edge:

<svg width='112.5' height='112.5' viewBox='0 0 225 225'>
    <use xlink:href='#node' x='75' y='75'/>
    <use xlink:href='#edge_ti' x='75' y = '75'/>
    <text font-size='{{small_font_size}}' x='150' y='200' dominant-baseline="central">{N, T}</text>
</svg>

Since iterators always point to a node, we never speak of an iterator being on the out edge of a node it is leaving. Rather, we always consider it on the in edge of the node that it points to. Visually, iterators will only ever be on the northwest or southeast edge of the node to which they point.

### Edge Flipping

Utility functions `adobe::leading_of(I)` and `adobe::trailing_of(I)` change the edge of an iterator to leading or trailing, respectively. It does not matter what edge the iterator was originally on, and the iterator will still point to the same node. These are free functions, not members of `adobe::forest<T>`. In this document we will represent these routines in pseudocode as such:

- `leading_of({P, X})` &rarr; `{P, L}`
- `trailing_of({N, X})` &rarr; `{N, T}`

### Iterator Adaptors

Although there is one iterator type for forest, there are several iterator adaptors that facilitate various methods of traversal. More will be said about these later.

### A note on `begin()` and `end()`

`begin()` and `end()` in a forest behave just like their equivalent member functions found in a typical standard library container.

- `forest<T>::end()` will always return `{R, T}`. This is true regardless of whether or not the forest is empty.

- `forest<T>::begin()` will always return `++{R, L}`. This follows the rules of the leading out edge explained above, meaning that it will either point to the first node in the forest (if it is not empty), or `end()` (if it is).

While iterators pointing to the root node $R$ are valid, they should never be dereferenced.

## Forest Traversal

### Fullorder

The traversal behavior of `adobe::forest<T>::iterator` is always fullorder. This means every node is visited twice: first, right before any of its children are visited (on the leading edge), and then again after the last child is visited (on the trailing edge). This behavior is recursive, and results in a depth-first traversal of the forest:

<img class='svg-img' src='{{site.baseurl}}/svg/fullorder_one.svg'/>

Here is the same diagram with the leading and trailing edges colorized green and red, respectively:

<img class='svg-img' src='{{site.baseurl}}/svg/fullorder_two.svg'/>

### Fullorder Traversal of $N$'s Subtree

To traverse a node $N$ and all of its descendants, the iterator range is:

- `leading_of({N, x})` &rarr; `first`
- `++trailing_of({N, x})` &rarr; `last`

<svg width='175' height='150' viewBox='0 0 350 300'>
    <use xlink:href='#edge_li' x='150' y = '150'/>
    <use xlink:href='#edge_lo' x='150' y = '150'/>
    <use xlink:href='#edge_ti' x='150' y = '150'/>
    <use xlink:href='#edge_to' x='150' y = '150'/>
    <use xlink:href='#node' x='150' y='150'/>
    <text font-size='{{small_font_size}}' x='0' y='30' dominant-baseline="central">{N, L} (first)</text>
    <text font-size='{{small_font_size}}' x='250' y='260' dominant-baseline="central">{N, T}</text>
    <text font-size='{{small_font_size}}' x='200' y='30' dominant-baseline="central">++{N, T} (last)</text>
</svg>

Note this technique does not work if $N==R$. In that case though, you'd be traversing the entire forest, in which case you can use `forest<T>::begin()` and `forest<T>::end()`.

### Preorder Traversal

A preorder traversal of the forest will visit every node once. During preorder traversal a parent will be visited before its children. We achieve preorder iteration by incrementing fullorder repeatedly, visiting a node only when the iterator is on a leading edge:

- `do ++{node, edge} while edge == trailing`

<img class='svg-img' src='{{site.baseurl}}/svg/preorder.svg'/>

### Postorder Traversal

A postorder traversal of the forest will visit every node once. During postorder traversal a parent will be visited after its children. We achieve postorder iteration by incrementing fullorder repeatedly, visiting a node only when the iterator is on a trailing edge:

- `do ++{node, edge} while edge == leading`

<img class='svg-img' src='{{site.baseurl}}/svg/postorder.svg'/>

### Child Traversal

A child traversal of a node P traverses only its immediate children; P itself is not visited. Child traversal is achieved by setting the edge of the iterator to trailing, then incrementing it:

- `++trailing_of({Cn, L})` &rarr; `{Cn+1, L}` (next child) or `{P, T}` (the end of the range)

<img class='svg-img' src='{{site.baseurl}}/svg/child_iteration.svg'/>

## Node Insertion

`forest<T>::insert` requires an iterator (where to insert) and a value (what to insert). The iterator is never invalidated during an insertion. `insert` returns a leading edge iterator to the new node.

### Leading Edge Insertion

When the location for insertion is on the leading edge of $N$ (that is, $I$ is `{N, L}`) the new node will be created as $N$'s new prior sibling. So:

<img class='svg-img' src='{{site.baseurl}}/svg/l_insert_before.svg'/>

becomes the following. Note that the iterator $I$ is unchanged:

<img class='svg-img' src='{{site.baseurl}}/svg/l_insert_after.svg'/>

In this case insert returns `{Sprior, L}`. Leading edge insertion can be used to repeatedly "push back" prior siblings of $N$. To repeatedly "push front" prior siblings of $N$, use the resulting iterator of `insert` as the next insertion position.

### Trailing Edge Insertion

When the location for insertion is on the trailing edge of $N$ (that is, $I$ is `{N, T}`) the new node will be created as $N$'s new last child. So:

<img class='svg-img' src='{{site.baseurl}}/svg/t_insert_before.svg'/>

becomes the following. Note that the iterator $I$ is unchanged:

<img class='svg-img' src='{{site.baseurl}}/svg/t_insert_after.svg'/>

In this case insert returns `{Clast, L}`. Trailing edge insertion can be used to repeatedly "push back" children of $N$. To repeatedly "push front" children of $N$, use the resulting iterator of `insert` as the next insertion position.

# Algorithms & Examples

## Detecting if $I_{node}$ is the First Child of its Parent

To understand how this works, lets start from the perspective of a parent node. Its first child (assuming it has one) will be pointed at after its leading edge iterator is incremented. In this specific situation, the iterator's edge will stay leading. (If the node has no children, the iterator will point to the trailing edge of the same node - the leaf node loop.)

Working backwards, then, you can start with a leading edge iterator to a node and decrement it. If the resulting iterator is still leading, you know you're pointing at the child's parent node- which also implies that child is the first of its parent. Otherwise, the iterator will be pointing to the trailing edge of its prior sibling (and so would not be the first child.)

More formally, given:

1. An iterator $I$ pointing to $I_{node}$
2. Its predecessor $H = std{\colon}{\colon}prev(I)$

If $I_{edge} == leading$ and $H_{edge} == leading$, then $I_{node}$ is the first child of $H_{node}$.

### Pseudocode

```
iterator i{/*...*/};
iterator h{--leading_of(i)};
bool     is_first_child{h.edge() == forest_leading_edge};
```

### Visually

Here is the case where the node is the first child:

<img class='svg-img' src='{{site.baseurl}}/svg/first_child_true.svg'/>

And here it is not:

<img class='svg-img' src='{{site.baseurl}}/svg/first_child_false.svg'/>

## Detecting if $I_{node}$ is the Last Child of its Parent

This one is pretty straightforward. Given a trailing edge iterator $I$ to the node $I_{node}$, incrementing it will either traverse to the next sibling or to its parent. In the former case, the edge of the iterator will flip from trailing to leading. In the latter case, the edge will stay trailing. Knowing this, the test is a check to see if the edge doesn't change.

More formally, given:

1. An iterator $I$ pointing to $I_{node}$
2. Its successor $J = std{\colon}{\colon}next(I)$

If $I_{edge} == trailing$ and $J_{edge} == trailing$, then $I_{node}$ is the last child of $J_{node}$.

### Pseudocode

```
iterator i{/*...*/};
iterator j{++trailing_of(i)};
bool     is_first_child{j.edge() == forest_trailing_edge};
```

### Visually

Here is the case where the node is the last child:

<img class='svg-img' src='{{site.baseurl}}/svg/last_child_true.svg'/>

And here it is not:

<img class='svg-img' src='{{site.baseurl}}/svg/last_child_false.svg'/>

