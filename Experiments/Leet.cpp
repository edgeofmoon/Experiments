#include <vector>
using namespace std;

class Solution {
public:
	double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2, int i1, int j1, int i2, int j2) {
		int midx = (nums1.size() + nums2.size() - 1) / 2;
		double ni1 = nums1[i1];
		double nj2 = nums2[j2];
		if (i1 + i2 >= midx){
			return ni1;
		}
		if (j1 + j2 <= midx){
			return nj2;
		}
		else{
			int midx1 = (i1 + j1) / 2;
			int midx2 = (i2 + j2) / 2;
			double mid1 = nums1[midx1];
			double mid2 = nums2[midx2];
			if (mid1 == mid2){
				return mid1;
			}
			else if (mid1>mid2){
				return findMedianSortedArrays(nums1, nums2, i1, midx1, midx2, j2);
			}
			else{
				return findMedianSortedArrays(nums1, nums2, midx1, j1, i2, midx2);
			}
		}

	}

	double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
		if (nums1.size() == 0) return nums2[nums2.size() / 2];
		if (nums2.size() == 0) return nums1[nums1.size() / 2];
		return findMedianSortedArrays(nums1, nums2, 0, nums1.size() - 1, 0, nums2.size() - 1);
	}
};

void main(){
	vector<int> v1(1, 1);
	vector<int> v2;
	v2.push_back(2);
	v2.push_back(3);
	v2.push_back(4);
	v2.push_back(5);
	Solution s;
	double x = s.findMedianSortedArrays(v1, v2);
	double z = 1.0;
}