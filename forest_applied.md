---
layout: page
title: 'Forest: Applications & Examples'
permalink: /forest-applied/
home-icon: tree
tab: Applications
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

# Applications, etc.
