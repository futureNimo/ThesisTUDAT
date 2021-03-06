/*    Copyright (c) 2010-2015, Delft University of Technology
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without modification, are
 *    permitted provided that the following conditions are met:
 *      - Redistributions of source code must retain the above copyright notice, this list of
 *        conditions and the following disclaimer.
 *      - Redistributions in binary form must reproduce the above copyright notice, this list of
 *        conditions and the following disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *      - Neither the name of the Delft University of Technology nor the names of its contributors
 *        may be used to endorse or promote products derived from this software without specific
 *        prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 *    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *    COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *    OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *    Changelog
 *      YYMMDD    Author            Comment
 *      120530    M.I. Ganeff       File created.
 *      121018    M.I. Ganeff       Added unit tests for computeSphereOfInfluence().
 *      121123    D. Dirkx          Added unit tests for computeSphereOfInfluence() function taking
 *                                  mass ratio.
 *      130227    D. Dirkx          Added unit test for isOrbitRetrograde() functions.
 *      130305    R.C.A. Boon       Replaced Eigen::VectorXd by basic_mathematics::Vector6d.
 *
 *    References
 *      Montebruck O, Gill E. Satellite Orbits, Corrected Third Printing, Springer, 2005.
 *      Bate R. Fundamentals of Astrodynamics, Courier Dover Publications, 1971.
 *      Wikipedia, Sphere of Influence, accesed 121018.
 *        http://en.wikipedia.org/wiki/Sphere_of_influence_(astrodynamics)
 *      Petit et al., IERS Conventions, IERS, 2010.
 *
 *    Notes
 *
 */

#define BOOST_TEST_MAIN

#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>

#include <Eigen/Core>

#include "Tudat/Mathematics/BasicMathematics/mathematicalConstants.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/orbitalElementConversions.h"

#include "Tudat/Astrodynamics/BasicAstrodynamics/missionGeometry.h"

namespace tudat
{
namespace unit_tests
{

//! Show the functionality of the unit tests.
BOOST_AUTO_TEST_SUITE( test_Mission_Geometry )

//! Unit test for shadow function (Sun,Earth).
BOOST_AUTO_TEST_CASE( testShadowFunctionForFullShadow )
{
    // Satellite totally blocked from solar radiation by the earth. Sun and satellite located on the
    // x-axis with the Earth in between. Altitude of satellite = 1000 km.

    const Eigen::Vector3d occultedBodyPosition = -149598000.0e3 * Eigen::Vector3d( 1.0, 0.0, 0.0 );
    const Eigen::Vector3d occultingBodyPosition = Eigen::Vector3d::Zero( );
    const double occultedBodyRadius = 6.96e8; // Siedelmann 1992.
    const double occultingBodyRadius = 6378.137e3; // WGS-84.

    const Eigen::Vector3d satellitePosition = ( occultingBodyRadius + 1.0e6 )
            * Eigen::Vector3d( 1.0, 0.0, 0.0 );

    // Compute shadow function.
    const double shadowFunction = mission_geometry::computeShadowFunction(
                occultedBodyPosition,
                occultedBodyRadius,
                occultingBodyPosition,
                occultingBodyRadius,
                satellitePosition );

    // Test values.
    BOOST_CHECK_EQUAL( 0.0, shadowFunction );
}

BOOST_AUTO_TEST_CASE( testShadowFunctionForFullLight )
{
    // Satellite totally subjected to sunlight. Satellite is located on the y-axis and the sun is
    // located on the x-axis. Altitude of satellite = 1000 km.

    const Eigen::Vector3d occultedBodyPosition = -149598000.0e3 * Eigen::Vector3d( 1.0, 0.0, 0.0 );
    const Eigen::Vector3d occultingBodyPosition = Eigen::Vector3d::Zero( );
    const double occultedBodyRadius = 6.96e8; // Siedelmann 1992.
    const double occultingBodyRadius = 6378.137e3; // WGS-84.

    const Eigen::Vector3d satellitePosition = ( occultingBodyRadius + 1.0e6 )
            * Eigen::Vector3d( 0.0, 1.0, 0.0 );

    // Compute shadow function.
    const double shadowFunction = mission_geometry::computeShadowFunction(
                occultedBodyPosition,
                occultedBodyRadius,
                occultingBodyPosition,
                occultingBodyRadius,
                satellitePosition );

    // Test values
    BOOST_CHECK_EQUAL( 1.0, shadowFunction );
}

BOOST_AUTO_TEST_CASE( testShadowFunctionForPartialShadow )
{
    // Satellite partially visible. Satellite is located between the locations of the previous
    // tests in penumbra. According to analytical derivations in Matlab the shadow function should
    // be around 0.4547. Altitude of satellite = 1000 km.

    const Eigen::Vector3d occultedBodyPosition = -149598000.0e3 * Eigen::Vector3d( 1.0, 0.0, 0.0 );
    const Eigen::Vector3d occultingBodyPosition = Eigen::Vector3d::Zero( );
    const double occultedBodyRadius = 6.96e8; // Siedelmann 1992.
    const double occultingBodyRadius = 6378.137e3; // WGS-84.

    Eigen::Vector3d satelliteDirection( 0.018, 1.0, 0.0 );
    satelliteDirection.normalize( );

    const Eigen::Vector3d satellitePosition = ( occultingBodyRadius + 1.0e3 ) * satelliteDirection;

    // Compute shadow function
    const double shadowFunction = mission_geometry::computeShadowFunction(
                occultedBodyPosition,
                occultedBodyRadius,
                occultingBodyPosition,
                occultingBodyRadius,
                satellitePosition );

    // Test values.
    BOOST_CHECK_CLOSE_FRACTION( 0.4547, shadowFunction, 0.001 );
}

//! Unit test for computation of radius of sphere of influence (Earth with respect to Sun).
BOOST_AUTO_TEST_CASE( testSphereOfInfluenceEarth )
{
    const double distanceEarthSun = 1.49597870700e11;  // (IERS, 2010)
    const double massEarth = 5.972186390142457e24;     // (IERS, 2010)
    const double massSun = 1.988415860572227e30;       // (IERS, 2010)

    // Test 1: test function taking masses.
    {
        // Calculate the sphere of influence of the Earth with respect to the Sun.
        const double sphereOfInfluenceEarth
                = mission_geometry::computeSphereOfInfluence(
                    distanceEarthSun, massEarth, massSun );

        // Test values (Wikipedia, Sphere of Influence)
        BOOST_CHECK_CLOSE_FRACTION( 9.25e8, sphereOfInfluenceEarth, 5.0e-4 );
    }

    // Test 2: test function taking mass ratio.
    {
        // Calculate the sphere of influence of the Earth with respect to the Sun.
        const double sphereOfInfluenceEarth
                = mission_geometry::computeSphereOfInfluence(
                    distanceEarthSun, massEarth / massSun );

        // Test values (Wikipedia, Sphere of Influence)
        BOOST_CHECK_CLOSE_FRACTION( 9.25e8, sphereOfInfluenceEarth, 5.0e-4 );
    }
}

//! Unit test for computation of radius of sphere of influence (Moon with respect to Earth).
BOOST_AUTO_TEST_CASE( testSphereOfInfluenceMoon )
{
    const double distanceMoonEarth = 3.84400e8;     // (Horizons, NASA)
    const double massMoon = 7.345811416686730e22;   // (IERS, 2010)
    const double massEarth = 5.972186390142457e24;  // (IERS, 2010)

    // Test 1: test function taking masses.
    {
        // Calculate the sphere of influence of the Moon with respect to the Earth.
        const double sphereOfInfluenceEarth
                = mission_geometry::computeSphereOfInfluence(
                    distanceMoonEarth, massMoon, massEarth );

        // Test values (Wikipedia, Sphere of Influence)
        BOOST_CHECK_CLOSE_FRACTION( 6.61e7, sphereOfInfluenceEarth, 2.0e-3 );
    }

    // Test 2: test function taking mass ratio.
    {
        const double sphereOfInfluenceMoon
                = mission_geometry::computeSphereOfInfluence(
                    distanceMoonEarth, massMoon / massEarth );

        // Test values (Wikipedia, Sphere of Influence)
        BOOST_CHECK_CLOSE_FRACTION( 6.61e7, sphereOfInfluenceMoon, 2.0e-3 );
    }
}

//! Unit test for testing for retrogradeness.
BOOST_AUTO_TEST_CASE( testIsOrbitRetrograde )
{
    using namespace orbital_element_conversions;
    using namespace mission_geometry;
    using namespace mathematical_constants;

    // Initialize test Kepler elements.
    basic_mathematics::Vector6d testKepler = Eigen::VectorXd::Zero( 6 );
    testKepler( semiMajorAxisIndex ) = 1.0e7;
    testKepler( eccentricityIndex ) = 0.1;
    testKepler( inclinationIndex ) = 50.0 / 180.0 * PI;
    testKepler( argumentOfPeriapsisIndex ) = 350.0 / 180.0 * PI;
    testKepler( longitudeOfAscendingNodeIndex ) = 15.0 / 180.0 * PI;
    testKepler( trueAnomalyIndex ) = 170.0 / 180.0 * PI;

    bool expectedIsOrbitRetrograde = false;

    bool calculatedIsOrbitRetrograde = true;

    // Test value for prograde orbit, not near boundary.
    {
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );

        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );
    }

    // Test value for prograde orbit, near zero boundary.
    {
        testKepler( inclinationIndex ) = std::numeric_limits< double >::epsilon( );
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );

        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );
    }

    // Test value for prograde orbit, near 90 degrees boundary.
    {
        testKepler( inclinationIndex ) = PI / 2.0 - std::numeric_limits< double >::epsilon( );
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );

        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );
    }

    // Test value for retrograde orbit, near 90 degrees boundary.
    {
        expectedIsOrbitRetrograde = 1;
        testKepler( inclinationIndex ) = PI / 2.0 + std::numeric_limits< double >::epsilon( );
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );

        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );
    }

    // Test value for retrograde orbit, not near boundary.
    {
        testKepler( inclinationIndex ) = 2.0;
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );

        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );
    }

    // Test value for retrograde orbit, near 180 degrees boundary.
    {
        testKepler( inclinationIndex ) = PI - std::numeric_limits< double >::epsilon( );
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );

        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
        BOOST_CHECK_EQUAL( expectedIsOrbitRetrograde, calculatedIsOrbitRetrograde );
    }

    // Check exception handling for inclinations out of range.
    bool isExceptionFound = false;
    testKepler( inclinationIndex ) = PI + 1.0;

    // Try to calculate retrogradeness.
    try
    {
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
    }
    // Catch the expected runtime error, and set the boolean flag to true.
    catch ( std::runtime_error )
    {
        isExceptionFound = true;
    }
    // Check value of flag.
    BOOST_CHECK( isExceptionFound );

    isExceptionFound = false;
    // Try to calculate retrogradeness.
    try
    {
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
    }
    // Catch the expected runtime error, and set the boolean flag to true.
    catch ( std::runtime_error )
    {
        isExceptionFound = true;
    }
    // Check value of flag.
    BOOST_CHECK( isExceptionFound );

    isExceptionFound = false;
    testKepler( inclinationIndex ) = -1.0;
    // Try to calculate retrogradeness.
    try
    {
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler );
    }
    // Catch the expected runtime error, and set the boolean flag to true.
    catch ( std::runtime_error )
    {
        isExceptionFound = true;
    }
    // Check value of flag.
    BOOST_CHECK( isExceptionFound );

    isExceptionFound = false;
    // Try to calculate retrogradeness.
    try
    {
        calculatedIsOrbitRetrograde = isOrbitRetrograde( testKepler( inclinationIndex ) );
    }
    // Catch the expected runtime error, and set the boolean flag to true.
    catch ( std::runtime_error )
    {
        isExceptionFound = true;
    }
    // Check value of flag.
    BOOST_CHECK( isExceptionFound );
}

BOOST_AUTO_TEST_SUITE_END( )

} // namespace unit_tests
} // namespace tudat
