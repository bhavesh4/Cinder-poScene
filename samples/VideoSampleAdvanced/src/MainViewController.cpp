#include "MainViewController.h"
#include "cinder/Log.h"

using namespace po::scene;

namespace sample
{
	ViewControllerRef ViewController::create()
	{
		return ViewControllerRef( new ViewController() );
	}

	ViewController::ViewController()
		: mNumMovies( 3 )
		, mIsControllerInPosition( false )
	{
	}

	void ViewController::viewDidLoad()
	{
		//  create and add the main player
		mPlayerController = PlayerController::create();
		mPlayerController->setAlignment( po::scene::Alignment::TOP_CENTER );
		mPlayerController->setPosition( ci::app::getWindowWidth() / 2, -mPlayerController->getHeight() / 2 ); // centered, just above screen
		mPlayerController->setAlpha( 0.f );
		getView()->addSubview( mPlayerController );

		//  set location for top/center of primary display
		mPrimaryDisplayerPosition = ci::vec2( ci::app::getWindowWidth() / 2, 50 );

		try {
			//  load the three videos
			ci::fs::path moviePath[3];
			moviePath[0] = ci::app::getAssetPath( "Placeholder_Video-RH3i7qONrT4.mp4" );
			moviePath[1] = ci::app::getAssetPath( "Placeholder_Video-ScMzIvxBSi4.mp4" );
			moviePath[2] = ci::app::getAssetPath( "Video-lBP2Ij86Ua4.mp4" );

			//  load the movies, create po::scene movie references, then create MovieThumb objects
			for( int i = 0; i < mNumMovies; i++ ) {
				auto qtMovie = ci::qtime::MovieGl::create( moviePath[i] );
				auto poVideoView = VideoViewGl::create();
				poVideoView->setMovieRef( qtMovie );
				mMoviePlayer[i] = MovieThumb::create( poVideoView );
				//mMovies[i]->setDrawBounds(true);
				getView()->addSubview( mMoviePlayer[i] );
			}

			setUpMovies();

		}
		catch( ... ) {
			std::cout << "Videos did not load successfully";
		}        
	}

	void ViewController::setUpMovies()
	{
		float thumbnailScale = 0.2f;
		float screenInterval = ci::app::getWindowWidth() / ( mNumMovies * 2 );

		for( int i = 0; i < mNumMovies; i++ ) {

            mMoviePlayer[i]->setAlignment( po::scene::Alignment::CENTER_CENTER );

			//  set scale and position of movie when it's the main one being displayed

			//  set scale of movie so it plays at width of 640 px (same as mPlayer width)
			float actualWidth = mMoviePlayer[i]->getUnderlyingMovie()->getMovieRef()->getWidth();
			float scale = mPlayerController->getWidth() / actualWidth;
            
//            CI_LOG_I("actualWidth: " << mMoviePlayer[i]->getUnderlyingMovie()->getWidth() << " ,scale: " << scale );
            
			mMoviePlayer[i]->setPlayerScale( ci::vec2( scale, scale ) );

			//  set position based on its height
			float yOffsetForPlayer = ( mMoviePlayer[i]->getUnderlyingMovie()->getHeight() * scale ) * 0.5;
			ci::vec2 playerPosition( mPrimaryDisplayerPosition.x, mPrimaryDisplayerPosition.y + yOffsetForPlayer );
			mMoviePlayer[i]->setPlayerPos( playerPosition );

			//  calculate the thumbnail scale, then set appropriate variable in mMovie object
			mMoviePlayer[i]->setThumbnailScale( mMoviePlayer[i]->getPlayerScale() * thumbnailScale );
			mMoviePlayer[i]->setScale( mMoviePlayer[i]->getThumbnailScale() );

			//  calculate the thumbnail position, then set appropriate variable in mMovie object
			float xPos = ( ( i * 2 ) + 1 ) * screenInterval;
			mMoviePlayer[i]->setThumbnailPos( ci::vec2( xPos, ci::app::getWindowHeight() * 0.8 ) );
			mMoviePlayer[i]->setPosition( mMoviePlayer[i]->getThumbnailPos() );

			//  add listeners
			mMoviePlayer[i]->getSignal( MouseEvent::Type::DOWN_INSIDE ).connect( std::bind( &ViewController::onThumbnailClick, this, std::placeholders::_1 ) );
			mMoviePlayer[i]->getSignalAnimationComplete().connect( std::bind( &ViewController::onAnimationComplete, this, std::placeholders::_1 ) );
		}
	}

	void ViewController::onThumbnailClick( MouseEvent& event )
	{
		ViewRef view = event.getSource();
		MovieThumbRef thumbnail = std::static_pointer_cast<MovieThumb>( view );

		for( int i = 0; i < mNumMovies; i++ ) {

			if( mMoviePlayer[i] == thumbnail ) {

				//  begin animation to primary displayer position, adjusted for center alignment
				mMoviePlayer[i]->animateToPlayer();
				animateControllerToPos( mMoviePlayer[i] );

				//  move primary movie to top position
				getView()->moveSubviewToFront( mMoviePlayer[i] );
			}
			else if( !mMoviePlayer[i]->getIsHome() ) {

				//  move other movies back if they're not in their home positions
				mMoviePlayer[i]->animateOutOfPlayerPosition();
			}
		}
	}

	void ViewController::onAnimationComplete( MovieThumbRef thumbnail )
	{
		mPlayerController->setPrimaryMovie( thumbnail->getUnderlyingMovie() );
	}

	void ViewController::animateControllerToPos( MovieThumbRef movie )
	{
		//  animate player controller to 50 px below the movie

		float x = mPlayerController->getPosition().x;
		//  find the height of the new movie when fully expanded
		float movieHeight = movie->getUnderlyingMovie()->getHeight() * movie->getPlayerScale().y;
		//  push the controller to 50 px below that
		float y = mPrimaryDisplayerPosition.y + movieHeight + 50;
		ci::vec2 newPos( x, y );

		if( !mIsControllerInPosition ) {
			ci::app::timeline().apply( &mPlayerController->getPositionAnim(), newPos, 2.f, ci::EaseOutBounce() );
		}
		else {
			ci::app::timeline().apply( &mPlayerController->getPositionAnim(), newPos, 2.f );
		}

		//  fade player in if it's transparent
		if( !mIsControllerInPosition ) {
			ci::app::timeline().apply( &mPlayerController->getAlphaAnim(), 1.f, 2.f );
			mIsControllerInPosition = true;
		}
	}
}
