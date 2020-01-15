# Necklace Map
# Notes based on paper (2015)
<!-- TODO(tvl) remove this file... -->

Input: set of polygons with values > 0, a necklace curve, and a necklace star-point (by default circle center if circular curve necklace).
* It may be important to check any references used (i.e. identifiers that link polygon and value).
* For usability, 0 values should be allowed in which case the polygon is ignored.
* The values should be normalized such that the sum is 1, which means that it is not allowed to have all values 0.
* Allow polygons with holes (ignore holes)? Allow regions with multiple disconnected polygons, and if yes, how to handle them? Multiple intervals per polygon => does this present algorithmic problems? Note that paper says each region has 1 interval, specifically the smallest interval that contains all allowed angles. Allow polygons with area 0?
* Is the original order of the input important (i.e. the order in which the polygons/values are presented)?
* What should happen when the line from necklace center through a region centroid also goes through another region's centroid? What if both centroids are on the necklace center?

Necklace intervals 2 methods: centroid, wedge.
* Take care to correctly handle intervals that include 0 degrees. Intervals not limited to [0,360]?
* Internally, always use radians, externally use degrees?
* What if polygon centroid is the necklace center?
* Leave an option for additional methods: the 2010 paper mentions a third method that is not implemented in the 2015 paper.
* Assumption: each interval < PI.
* Fixed order output (O(n*log(n))), versus arbitrary order output exact (O(n*log(n) + n^2*K*4^K)), versus arbitrary order output heuristic (O(n*log(n) + n*K*2^K)).

Output: positions of necklace glyph centers (assuming circular glyphs) and global scale factor. The scale factor times the squared root of each value gives the glyph radius.
* No glyph can cover necklace 'center'.

Multiple necklaces: compute per necklace. However, the algorithms do not check whether the glyphs of one necklace overlap the glyphs of another. Perhaps this can be addressed by indicating disallowed regions on a necklace? Or at least by checking whether the necklaces overlap or are close to eachother?



"Solutions to the one-dimensional problem provide a very good approximation for the original necklace map problem." This indicates that we have to do some work after computing the optimal solution for the 1D problem.

"Fixed-Order, where an order for the symbols on the necklace is given". This suggests that the order is not determined by the algorithm itself. However, we may actually still want to do that.


Necklace Element vectors versus Element feature vectors.
* Pro element vectors: all features grouped and e.g. can be sorted together.
* Pro feature vectors: can specify per feature whether it is constant.


Notes on Java implementation
* Entry point: FlowCircle.java applet.
  * This calls DrawPanel.java methods to refresh drawing and to initiate computations.
    * This calls Optimizer.java: main functional part (that implements the dynamic programming part).
  * [not true:] Immediately calculates Necklace Maps for all datasets in the maps directory (mainPanel.loadMap(curMap);). [instead does this for this first map found.]
    * Note that "Map.loadMap()" loads in the data, but does not yet calculate the necklace maps. This is done in DrawPanel.setDataset(interval algorithm ID).
    * Algorithm.computeRanges() computes the valid intervals according to the specified Algorithm (AlgCentroid, or AlgWedge with some weighting factor).
    * Optimizer.computeOptSize(...) computes the scale factor. Note that the optimum is calculated per necklace, but then the minimum over all necklaces must be taken.
      - Apparently, there is an error in the fixed order implementation of the optimum scaling factor. The 2015 paper is correct though.
    * Optimizer.optFixedOrder(...) computes the positions of the glyph centers. Note that the fixed order is based on the interval centers for the centroid intervals and based on the interval start for wedge intervals.
  * The applet allows multiple necklaces by adding "_#" to the polygon/value IDs (which are otherwise 2-character country/region identifiers).
  * Note that no check seems to be made whether the glyphs of different nacklaces overlap.
  * Recalculates when using the "Buffer size" slider, but not the "Force focus" slider (this does not change the scale factor, but it does change the glyph centers). Finally, the "Zoom" slider changes the screen positions, but not the relative positions of the glyphs and base map.
* Code contains a lot of surrounding nonsense, unused experimental code, and things such as a 'water waves simulater".

