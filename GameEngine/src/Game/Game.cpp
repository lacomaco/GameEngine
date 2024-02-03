#include "Game.h"
#include <SDL.h>
#include <GL/glew.h>
#include "../Util/Shader.h"

Game::Game() {
	mWindow = nullptr;
	mIsRunning = true;

}

bool Game::Initialize() {
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);

	if (sdlResult != 0) {
		SDL_Log("SDL 초기화 실패: %s", SDL_GetError());
		return false;
	}

	mWindow = SDL_CreateWindow(
		"My Game Practices",
		100,
		100,
		1024,
		768,
		SDL_WINDOW_OPENGL
	);

	if (!mWindow) {
		SDL_Log("윈도우 생성 실패 %s", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);

	context = SDL_GL_CreateContext(mWindow);

	if (!context) {
		SDL_Log("GL Context 생성 실패! %s", SDL_GetError());
	}

	if (glewInit() != GLEW_OK) {
		SDL_Log("GLEW 초기화 실패!");
		return false;
	}

	glViewport(0, 0, 1024, 768);
	glCullFace(GL_BACK);
	// 기본값 CW, 근데 우리는 윈도우에서 사용하기 때문에
	// DirectX와 같은 CCW로 설정하였음.
	glFrontFace(GL_CCW);

	SetOpenGL();

	return true;
}

void Game::Shutdown() {
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
	SDL_GL_DeleteContext(context);
}

void Game::RunLoop() {
	while (mIsRunning) {
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			//  윈도우의 x 버튼을 누르면 게임 종료
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	// 키보드 키가 눌렸는지 감지
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE]) {
		mIsRunning = false;
	}
}

void Game::UpdateGame() {

	/*
	* fps를 60으로 제한한다. (60FPS면 얼추 16.6ms이다.)
	* 
	* PS: fps를 60으로 제한하는 이유
	* 
	* 물리 계산을 매번 Update할때 프레임이 달라지면 물리 게산 결과도 달라진다.
	* 여기선 가장 쉬운 방법으로 최대 fps를 60으로 제한 거는 방법으로 해결함
	*/
	while (!SDL_TICKS_PASSED(SDL_GetTicks(),mTicksCount + 16));

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	mTicksCount = SDL_GetTicks();

	// 델타 시간이 너무 크면 게임이 멈추는 것을 방지
	/*
	* 이 로직이 필요한 이유.
	* 만약 sleep을 걸었거나, 디버거로 시스템을 잠구고 다시 실행시키면
	* deltaTime이 크게 불어나는 경우가 발생한다.
	* 이를 방지하기 위해서 deltaTime이 0.05f를 넘지 않도록 제한한다.
	* 60fps로 제한을 걸어두었기 때문에 일반적인 상황에선 deltaTime은
	* 0.0166f 정도가된다.
	*/
	if (deltaTime > 0.05f) {
		deltaTime = 0.05f;
	}
}

void Game::GenerateOutput() {
	auto program = Shader::getInstance()->getShaderProgram("triangle");
	glUseProgram(program);
	glBindVertexArray(VAO);
	// 하얀색으로 초기화.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	* 6: 인덱스 갯수
	* GL_UNSIGNED_INT: 인덱스 타입
	* 0: 인덱스 배열 시작 위치
	*/
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SDL_GL_SwapWindow(mWindow);
}

void Game::SetOpenGL() {
	// 셰이더 로드

	auto shader = Shader::getInstance();
	// 초기 로드
	auto program = shader->loadShaderProgram(
		"./shader/triangle-vertex.glsl",
		"./shader/triangle-fragment.glsl",
		"triangle"
	);

	/*
	* void glGenBuffers(GLsizei n, GLuint *buffers);
	* 
	* 1 <- 버퍼 객체갯수.
	*/
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/*
	* 이 코드의 의미.
	* 
	* GL_ARRAY_BUFFER에 바인딩된 Buffer OBJECT에
	* vertices.size() * sizeof(float) 만큼의 메모리를 할당하고
	* vertices.data()의 데이터를 복사한다.
	* 또한 GL_STATIC_DRAW로 설정하여 데이터가 변하지 않음을 명시한다.
	*/
	glBufferData(
		GL_ARRAY_BUFFER, 
		vertices.size() * sizeof(float),
		vertices.data(), 
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		indices.size() * sizeof(unsigned int),
		indices.data(),
		GL_STATIC_DRAW
	);
}

void Game::AddActor(Actor* actor)
{
	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActors.emplace_back(actor);
	}

}

void Game::RemoveActor(Actor* actor)
{
	auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
	if (iter != mPendingActors.end())
	{
		std::iter_swap(iter, mPendingActors.end() - 1);
		mPendingActors.pop_back();
	}

	iter = std::find(mActors.begin(), mActors.end(), actor);
	if (iter != mActors.end())
	{
		std::iter_swap(iter, mActors.end() - 1);
		mActors.pop_back();
	}
}
