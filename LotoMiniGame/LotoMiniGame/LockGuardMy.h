namespace myLG {
	template<class T>
	class LockGuardMy {
		T * mutex;

	public:
		LockGuardMy(T * mutex) {
			this->mutex = mutex;

			this->mutex->lock();
		}

		~LockGuardMy() {
			this->mutex->unlock();
		}
	};
}